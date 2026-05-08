#include "../include/traceroute.h"

void free_ctx(t_context *ctx) {
    free(ctx->host_addr);
    free(ctx->active_probes);

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

    ctx->active_probes = malloc(sizeof(t_probe *) * ctx->sim_queries);
    if (!ctx->active_probes) {
        fprintf(stderr, "error: failed to allocate active probes: %s\n", strerror(errno));
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
    t_probe *probe = malloc(sizeof(t_probe));
    if (!probe) {
        fprintf(stderr, "error: failed to allocate probe: %s\n", strerror(errno));
        return -1; 
    }

    t_hop *hop = &ctx->hops[ctx->current_ttl - 1];
    size_t hop_query_idx = (ctx->current_port - ctx->port) % ctx->queries;
    t_query *query = &hop->queries[hop_query_idx];

    probe->dst_port = ctx->current_port++;
    probe->ttl = hop_query_idx != (ctx->queries - 1) ? ctx->current_ttl : ctx->current_ttl++;

    query->req = probe;

    struct sockaddr_in *host_addr = (struct sockaddr_in *)ctx->host_addr;
    host_addr->sin_port = htons(probe->dst_port);

    if (setsockopt(ctx->udp_sock->fd , IPPROTO_IP, IP_TTL, &probe->ttl, sizeof(probe->ttl)) == -1) {
        fprintf(stderr, "error: failed to set socket TTL: %s\n", strerror(errno));
        return -1;
    }

    size_t payload_len = ctx->packet_len - sizeof(struct iphdr) - sizeof(struct udphdr);
    uint8_t *payload = malloc(sizeof(uint8_t) * payload_len);
    if (!payload) {
        fprintf(stderr, "error: failed to allocate payload: %s\n", strerror(errno));
        return -1;
    }

    if (gettimeofday(&probe->send_time, NULL) == -1) {
        fprintf(stderr, "error: failed to retrieve timestamp: %s\n", strerror(errno));
        return -1;
    }

    ssize_t bytes_sent = sendto(ctx->udp_sock->fd, payload, payload_len, 0, (struct sockaddr *)host_addr, ctx->host_addr_len);
    if (bytes_sent == -1) {
        fprintf(stderr, "error: failed to send probe: %s\n", strerror(errno));
        return -1;
    }

    free(payload);
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

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(ctx->icmp_sock->fd , &read_fds);
    
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 50000;
    
    char *host_sock_addr = inet_ntoa(((struct sockaddr_in *)ctx->host_addr)->sin_addr);
    printf("traceroute to %s (%s), %ld hops max, %ld bytes packets\n", host_sock_addr, host_sock_addr, ctx->max_ttl, ctx->packet_len);
    send_probe(ctx);

    while (1) {
        int nfds = select(ctx->icmp_sock->fd  + 1, &read_fds, NULL, NULL, &timeout);
        if (nfds == -1) {
            fprintf(stderr, "error: select failed: %s\n", strerror(errno));
        } else if (nfds > 0 && FD_ISSET(ctx->icmp_sock->fd , &read_fds)) {
            process_icmp_reply(ctx);
        }
    }

    free_ctx(ctx);
    return 0;
}
