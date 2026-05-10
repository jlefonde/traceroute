#include "../include/traceroute.h"

void free_ctx(t_context *ctx) {
    free(ctx->host_addr);
    free(ctx->active_queries);

    free_socket(ctx->udp_sock);
    free_socket(ctx->icmp_sock);

    if (ctx->hops) {
        for (size_t i = 0; i < ctx->max_ttl; i++) {
            free(ctx->hops[i].queries);
        }

        free(ctx->hops);
    }

    free(ctx);
}

static t_option *init_options() {
    static t_option options[] = {
        {
            .short_opt = 'h',
            .long_opt = "help",
            .description = "Print this help message and exit",
            .has_argument = false,
        },
    };

    return options;
}

t_context *init_ctx() {
    t_context *ctx = malloc(sizeof(t_context));
    if (!ctx) {
        fprintf(stderr, "error: failed to init context: %s\n", strerror(errno));
        return NULL;
    }

    ctx->options = init_options();
    ctx->host_addr = NULL;
    ctx->port = 33434;
    ctx->current_port = ctx->port;
    ctx->current_ttl = 1;
    ctx->max_ttl = 30;
    ctx->queries = 3;
    ctx->sim_queries = 16;
    ctx->timeout = 3;
    ctx->packet_len = 60;

    ctx->udp_sock = init_udp_socket();
    if (!ctx->udp_sock) {
        free_ctx(ctx);
        return NULL;
    }

    ctx->icmp_sock = init_icmp_socket();
    if (!ctx->icmp_sock) {
        free_ctx(ctx);
        return NULL;
    }

    ctx->active_queries = malloc(sizeof(t_query *) * ctx->sim_queries);
    if (!ctx->active_queries) {
        fprintf(stderr, "error: failed to allocate active queries: %s\n", strerror(errno));
        free_ctx(ctx);
        return NULL;
    }

    ctx->hops = malloc(sizeof(t_hop) * ctx->max_ttl);
    if (!ctx->hops) {
        fprintf(stderr, "error: failed to allocate hops: %s\n", strerror(errno));
        free_ctx(ctx);
        return NULL;
    }

    for (size_t i = 0; i < ctx->max_ttl; i++) {
        ctx->hops[i].queries = malloc(sizeof(t_query) * ctx->queries);
        if (!ctx->hops[i].queries) {
            fprintf(stderr, "error: failed to allocate hops' queries: %s\n", strerror(errno));
            free_ctx(ctx);
            return NULL;
        }
    }

    return ctx;
}

int send_probe(t_context *ctx) {
    if (ctx->current_ttl > ctx->max_ttl) {
        return 0;
    }

    // TODO: make sure the rest of the probe queries are send not only one
    if (ctx->unreached_port_ttl > 0 && ctx->current_ttl > ctx->unreached_port_ttl) {
        return 0;
    }

    int active_query_idx = -1;
    for (size_t i = 0; i < ctx->sim_queries; i++) {
        if (ctx->active_queries[i] == NULL) {
            active_query_idx = i;
            break;
        }
    }

    if (active_query_idx == -1) {
        return 0;
    }

    t_probe *probe = malloc(sizeof(t_probe));
    if (!probe) {
        return -1; 
    }

    size_t hop_query_idx = (ctx->current_port - ctx->port) % ctx->queries;
    t_query *query = &ctx->hops[ctx->current_ttl - 1].queries[hop_query_idx];
    
    probe->dst_port = ctx->current_port;
    probe->ttl = ctx->current_ttl;
    query->req = probe;
    ctx->active_queries[active_query_idx] = query;

    struct sockaddr_in *host_addr = (struct sockaddr_in *)ctx->host_addr;
    host_addr->sin_port = htons(probe->dst_port);

    if (setsockopt(ctx->udp_sock->fd , IPPROTO_IP, IP_TTL, &probe->ttl, sizeof(probe->ttl)) == -1) {
        return -1;
    }

    size_t payload_len = (ctx->packet_len > sizeof(struct iphdr) - sizeof(struct udphdr)) ? ctx->packet_len - sizeof(struct iphdr) - sizeof(struct udphdr) : 0;
    uint8_t *payload = malloc(sizeof(uint8_t) * payload_len);
    if (!payload) {
        return -1;
    }

    if (gettimeofday(&probe->send_time, NULL) == -1) { 
        free(payload);
        return -1;
    }

    ssize_t bytes_sent = sendto(ctx->udp_sock->fd, payload, payload_len, 0, (struct sockaddr *)host_addr, ctx->host_addr_len);
    if (bytes_sent == -1) {
        free(payload);
        return -1;
    }

    ctx->current_port++;
    if (hop_query_idx == ctx->queries - 1) {
        ctx->current_ttl++;
    }

    free(payload);
    return 1;
}

const char *get_icmp_dest_unreach_annotation(uint8_t code) {
    switch (code) {
        case ICMP_NET_UNREACH: return "!N";
        case ICMP_HOST_UNREACH: return "!H";
        case ICMP_PROT_UNREACH: return "!P";
        case ICMP_FRAG_NEEDED: return "!F";
        case ICMP_SR_FAILED: return "!S";
        case ICMP_PKT_FILTERED: return "!X";
        case ICMP_PREC_VIOLATION: return "!V";
        case ICMP_PREC_CUTOFF: return "!C";
        case ICMP_PORT_UNREACH: return "";
        default: return NULL;
    }
}

static void update_active_queries(t_context *ctx) {
    struct timeval now;
    gettimeofday(&now, NULL);

    for (size_t i = 0; i < ctx->sim_queries; i++) {
        t_query *query = ctx->active_queries[i];
        if (!query) {
            continue;
        }

        if (query->rep) {
            ctx->active_queries[i] = NULL;
        } else if (get_elapsed_ms(query->req->send_time, now) >= (double)ctx->timeout * 1000.0) {
            query->rtt = -1.0;
            ctx->active_queries[i] = NULL;
        }
    }
}

int main(int argc, char **argv) {
    if (getuid() != ROOT_UID) {
        fprintf(stderr, "error: root privileges are required\n");
        return 1;
    }

    t_context *ctx = init_ctx();
    if (!ctx) {
        return 1;
    }

    int err = parse_args(ctx, argc, argv);
    if (err != 0) {
        return 1;
    }
    
    char *host_ip = inet_ntoa(((struct sockaddr_in *)ctx->host_addr)->sin_addr);
    printf("traceroute to %s (%s), %ld hops max, %ld bytes packets\n", ctx->host_str, host_ip, ctx->max_ttl, ctx->packet_len);

    fd_set read_fds; 
    struct timeval timeout;
    
    while (1) {
        while (send_probe(ctx) > 0);

        FD_ZERO(&read_fds);
        FD_SET(ctx->icmp_sock->fd, &read_fds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;

        if (select(ctx->icmp_sock->fd + 1, &read_fds, NULL, NULL, &timeout) > 0) {
            process_icmp_reply(ctx);
        }
        
        update_active_queries(ctx);

        if (ctx->unreached_port_ttl > 0 && ctx->next_hop >= ctx->unreached_port_ttl) {
            break;
        }

        if (ctx->next_hop >= ctx->max_ttl) {
            break;   
        }
    }

    free_ctx(ctx);
    return 0;
}
