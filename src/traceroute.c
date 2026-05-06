#include "../include/traceroute.h"

void free_ctx(t_context *ctx) {
    free(ctx->host_addr);
    free(ctx->active_probes);

    if (ctx->hops) {
        for (size_t i = 0; i < ctx->max_ttl; i++) {
            free(ctx->hops[i].replies);
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

t_context *init_ctx(int argc, char **argv) {
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

    if (argc < 2) {
        print_helper(ctx->options);
        free_ctx(ctx);
        return NULL;
    }

    int err = parse_args(ctx, argc, argv);
    if (err == 2) {
        print_helper(ctx->options);
        free_ctx(ctx);
        exit(EXIT_SUCCESS);
    } else if (err != 0) {
        free_ctx(ctx);
        return NULL;
    }

    ctx->active_probes = malloc(sizeof(t_probe) * ctx->sim_queries);
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
        ctx->hops[i].replies = malloc(sizeof(t_icmp) * ctx->queries);
        if (!ctx->hops[i].replies) {
             fprintf(stderr, "error: failed to allocate hops' replies: %s\n", strerror(errno));
             free_ctx(ctx);
             return NULL;
        }
    }

    return ctx;
}

int parse_icmp_payload(t_icmp *icmp) {
    if (icmp->reply_len < (ssize_t)(icmp->reply_offset + sizeof(struct iphdr))) {
        return -1;
    }

    icmp->data.ip_hdr = (struct iphdr *)(icmp->reply + icmp->reply_offset);
    size_t inner_ip_len = icmp->data.ip_hdr->ihl * 4;

    if (icmp->reply_len < (ssize_t)(icmp->reply_offset + inner_ip_len + 8)) {
        return -1;
    }

    icmp->reply_offset += inner_ip_len; 
    icmp->data.udp_hdr = (struct udphdr *)(icmp->reply + icmp->reply_offset);
 
    return 0;
}

int find_active_probes_free_slot(t_context *ctx) {
    for (int i = 0; i < ctx->sim_queries; i++) {
        if (!ctx->active_probes[i].is_active) {
            return i;
        }
    }

    return -1;
}

int send_probe(t_context *ctx) {
    int active_probes_idx = find_active_probes_free_slot(ctx);
    if (active_probes_idx == -1) {
        return 2;
    }

    t_probe probe = {
        .is_active = true,
        .ttl = ctx->current_ttl,
        .dst_port = ctx->current_port++,
    };

    struct sockaddr_in *host_addr = (struct sockaddr_in *)ctx->host_addr;
    host_addr->sin_port = htons(probe.dst_port);

    if (setsockopt(ctx->send_sock_fd, IPPROTO_IP, IP_TTL, &probe.ttl, sizeof(probe.ttl)) == -1) {
        fprintf(stderr, "error: failed to set socket TTL: %s\n", strerror(errno));
        return -1;
    }

    size_t payload_len = ctx->packet_len - sizeof(struct iphdr) - sizeof(struct udphdr);
    uint8_t *payload = malloc(sizeof(uint8_t) * payload_len);
    if (!payload) {
        fprintf(stderr, "error: failed to allocate payload: %s\n", strerror(errno));
        return -1;
    }

    ft_memset(payload, 0, payload_len);
    if (gettimeofday(&probe.send_time, NULL) == -1) {

    }

    ft_memcpy(payload, &probe.send_time, sizeof(struct timeval));

    ssize_t bytes_sent = sendto(ctx->send_sock_fd, payload, payload_len, 0, (struct sockaddr *)host_addr, ctx->host_addr_len);
    if (bytes_sent == -1) {
        fprintf(stderr, "error: failed to send probe: %s\n", strerror(errno));
        free_ctx(ctx);
        return -1;
    }

    ctx->active_probes[active_probes_idx] = probe;
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

int parse_icmp_hdr(t_icmp *icmp) {
    if (icmp->reply_len < sizeof(struct iphdr)) {
        return -1;
    }

    icmp->ip_hdr = (struct iphdr *)icmp->reply;
    size_t actual_ip_len = icmp->ip_hdr->ihl * 4;

    if (icmp->reply_len < actual_ip_len + sizeof(struct icmphdr)) {
        return -1;
    }

    icmp->hdr = (struct icmphdr *)(icmp->reply + actual_ip_len);
    icmp->reply_offset = actual_ip_len + sizeof(struct icmphdr);

    return 0;
}

int main(int argc, char **argv) {
    t_context *ctx = init_ctx(argc, argv);
    if (!ctx) {
        return -1;
    }

    ctx->send_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (ctx->send_sock_fd == -1) {
        fprintf(stderr, "error: failed to create send socket: %s\n", strerror(errno));
        free_ctx(ctx);
        return -1;
    }

    ctx->recv_sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (ctx->recv_sock_fd == -1) {
        fprintf(stderr, "error: failed to create recv socket: %s\n", strerror(errno));
        free_ctx(ctx);
        return -1;
    }

    struct sockaddr send_sock_addr;
    ft_memset(&send_sock_addr, 0, sizeof(send_sock_addr));

    send_sock_addr.sa_family = AF_INET;
    if (bind(ctx->send_sock_fd, (struct sockaddr *) &send_sock_addr, sizeof(send_sock_addr)) == -1) {
        fprintf(stderr, "error: failed to bind send socket: %s\n", strerror(errno));
        free_ctx(ctx);
        return -1;
    }

    if (setsockopt(ctx->send_sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
        fprintf(stderr, "error: failed to set send socket options: %s\n", strerror(errno));
        free_ctx(ctx);
        return -1;
    }

    unsigned int icmp_filter = ~0U;
    icmp_filter &= ~(1U << ICMP_DEST_UNREACH);
    icmp_filter &= ~(1U << ICMP_TIME_EXCEEDED);

    if (setsockopt(ctx->recv_sock_fd, SOL_RAW, ICMP_FILTER, &icmp_filter, sizeof(icmp_filter)) == -1) {
        fprintf(stderr, "error: failed to set recv socket options: %s\n", strerror(errno));
        free_ctx(ctx);
        return -1;
    }

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(ctx->recv_sock_fd, &read_fds);
    
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 50000;

    char *host_sock_addr = inet_ntoa(((struct sockaddr_in *)ctx->host_addr)->sin_addr);
    printf("traceroute to %s (%s), %ld hops max, %ld bytes packets\n", host_sock_addr, host_sock_addr, ctx->max_ttl, ctx->packet_len);
    send_probe(ctx);

    while (1) {
        int nfds = select(ctx->recv_sock_fd + 1, &read_fds, NULL, NULL, &timeout);
        if (nfds == -1) {
            fprintf(stderr, "error: select failed: %s\n", strerror(errno));
        } else if (nfds > 0) {
            if (FD_ISSET(ctx->recv_sock_fd, &read_fds)) {
                t_icmp *icmp = &ctx->icmp;
                ssize_t bytes_received = recvfrom(ctx->recv_sock_fd, icmp->reply, sizeof(icmp->reply), 0, NULL, NULL);

                if (bytes_received > 0) {
                    icmp->reply_len = bytes_received;

                    if (parse_icmp_hdr(icmp) != 0 || parse_icmp_payload(icmp) != 0) {
                        return -1;
                    }

                    printf("src=%d, dst=%d\n", ntohs(icmp->data.udp_hdr->source),  ntohs(icmp->data.udp_hdr->dest));
                }
            }
        }
    }

    free_ctx(ctx);
    return 0;
}
