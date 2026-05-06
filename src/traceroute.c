#include "../include/traceroute.h"

void free_ctx(t_context *ctx) {
    free(ctx->host_addr);
    free(ctx->active_probes);

    if (ctx->hops) {
        free(ctx->hops->probes);
        free(ctx->hops);
    }

    free(ctx);
}

static t_option *get_option_short(t_option *options, char opt) {
    for (size_t i = 0; options[i].short_opt; i++) {
        if (options[i].short_opt == opt) {
            return &options[i];
        }
    }

    return NULL;
}

static t_option *get_option_long(t_option *options, char *opt) {
    for (size_t i = 0; options[i].long_opt; i++) {
        if (options[i].long_opt[0] && ft_strcmp(options[i].long_opt, opt) == 0) {
            return &options[i];
        }
    }

    return NULL;
}

static void print_helper(t_option *options) {
    printf("usage: ./ft_traceroute <host> [options]\n");
    printf("options:\n");

    for (size_t i = 0; options[i].short_opt || (options[i].long_opt && options[i].long_opt[0]); i++) {
        int len = 0;
        
        if (options[i].short_opt && options[i].long_opt && options[i].long_opt[0]) {
            len = printf("  -%c, --%s", options[i].short_opt, options[i].long_opt);
        } else if (options[i].short_opt) {
            len = printf("  -%c", options[i].short_opt);
        } else if (options[i].long_opt && options[i].long_opt[0]) {
            len = printf("      --%s", options[i].long_opt);
        }

        if (options[i].has_argument && options[i].data.arg.meta) {
            len += printf(" <%s>", options[i].data.arg.meta);
        }

        if (len < 24) {
            printf("%*s", 24 - len, "");
        } else {
            printf(" ");
        }

        printf("%s\n", options[i].description ? options[i].description : "");
    }
}

static bool check_helper(char **argv) {
    for (char **arg = argv + 1; *arg; arg++) {
        if ((*arg)[0] != '-') {
            continue;
        }

        if ((*arg)[1] != '-' && ft_strchr(*arg + 1, 'h') != NULL) {
            return true;
        } else if (ft_strcmp(*arg + 2, "help") == 0) {
            return true;
        }
    }

    return false;
}

static int parse_short_option(t_context *ctx, char ***argv) {
    char *curr_str = **argv;
    int len = ft_strlen(curr_str);

    for (int i = 1; i < len; i++) {
        int option_idx = -1;
        char short_opt = curr_str[i];

        t_option *option = get_option_short(ctx->options, short_opt);
        if (!option) {
            fprintf(stderr, "error: bad option '-%c'\n", short_opt);
            return 1;
        }

        if (!option->has_argument) {
            option->data.flag.is_set = true;
            continue;
        } 

        if (i == len - 1 && (*argv)[1]) {
            (*argv)++;
            option->data.arg.value = **argv;
        }
        else {
            fprintf(stderr, "error: option '-%c' requires an argument\n", short_opt);
            return 1;
        }
    }

    return 0;
}

static int parse_long_option(t_context *ctx, char ***argv) {
    int option_idx = -1;
    char *long_opt = (**argv) + 2;

    t_option *option = get_option_long(ctx->options, long_opt);
    if (!option) {
        fprintf(stderr, "error: bad option '--%s'\n", long_opt);
        return 1;
    }

    if (!option->has_argument) {
        option->data.flag.is_set = true;
    }
    else if ((*argv)[1]) {
        (*argv)++;
        option->data.arg.value = **argv;
    }
    else {
        fprintf(stderr, "error: option '--%s' requires an argument\n", long_opt);
        return 1;
    }

    return 0;
}

static int set_host_address(t_context *ctx, char *host_str) {
    if (!host_str) {
        fprintf(stderr, "error: missing \"host\" argument\n");
        return 1;
    }

    struct addrinfo *res;
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM,
        .ai_flags = AI_ADDRCONFIG,
    };

    int err_code = getaddrinfo(host_str, NULL, &hints, &res);
    if (err_code != 0) {
        fprintf(stderr, "error: '%s': %s\n", host_str, gai_strerror(err_code));
        return 1;
    }

    ctx->host_addr_len = res->ai_addrlen;
    ctx->host_addr = malloc(ctx->host_addr_len);
    if (!ctx->host_addr) {
        fprintf(stderr, "error: failed to allocate host address: %s\n", strerror(errno));
        freeaddrinfo(res);
        return 1;
    }

    if (res->ai_family == AF_INET) {
        ctx->packet_len = 60;
    } else if (res->ai_family == AF_INET6) {
        ctx->packet_len = 80;
    }

    ft_memcpy(ctx->host_addr, res->ai_addr, ctx->host_addr_len);

    
    freeaddrinfo(res);
    return 0;
}

static int parse_args(t_context *ctx, int argc, char **argv) {
    if (check_helper(argv)) {
        return 2;
    }

    char *host_str = NULL;
    for (char **arg = argv + 1; *arg; arg++) {
        int err = 0;

        if ((*arg)[0] == '-' && (*arg)[1] && (*arg)[1] != '-') {
            err = parse_short_option(ctx, &arg);
        }
        else if ((*arg)[0] == '-' && (*arg)[1] == '-' && (*arg)[2] && (*arg)[2] != '-') {
            err = parse_long_option(ctx, &arg);
        }
        else if (!host_str) {
            host_str = *arg;
        } else {
            fprintf(stderr, "error: extra argument '%s'\n", *arg);
            return 1;
        }

        if (err != 0) {
            return 1;
        }
    }
    
    if (set_host_address(ctx, host_str) != 0) {
        return 1;
    }

    return 0;
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

    ctx->hops->probes = malloc(sizeof(t_probe) * ctx->queries);
    if (!ctx->hops) {
        fprintf(stderr, "error: failed to allocate hops' probes: %s\n", strerror(errno));
        free_ctx(ctx);
        return NULL;
    }

    return ctx;
}

int parse_icmp_payload(t_icmp *icmp) {
    if (icmp->reply_len < (ssize_t)(icmp->reply_offset + sizeof(struct iphdr))) {
        return 1;
    }

    icmp->data.ip_hdr = (struct iphdr *)(icmp->reply + icmp->reply_offset);
    size_t inner_ip_len = icmp->data.ip_hdr->ihl * 4;

    if (icmp->reply_len < (ssize_t)(icmp->reply_offset + inner_ip_len + 8)) {
        return 1;
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

    printf("%s\n", inet_ntoa(((struct sockaddr_in *)ctx->host_addr)->sin_addr));
    printf("%d\n", probe.dst_port);

    if (setsockopt(ctx->send_sock_fd, IPPROTO_IP, IP_TTL, &probe.ttl, sizeof(probe.ttl)) == -1) {
        fprintf(stderr, "error: failed to set socket TTL: %s\n", strerror(errno));
        return 1;
    }

    size_t payload_len = ctx->packet_len - sizeof(struct iphdr) - sizeof(struct udphdr);
    uint8_t *payload = malloc(sizeof(uint8_t) * payload_len);
    if (!payload) {
        fprintf(stderr, "error: failed to allocate payload: %s\n", strerror(errno));
        return 1;
    }

    ft_memset(payload, 0, payload_len);
    if (gettimeofday(&probe.send_time, NULL) == -1) {

    }

    ft_memcpy(payload, &probe.send_time, sizeof(struct timeval));

    ssize_t bytes_sent = sendto(ctx->send_sock_fd, payload, payload_len, 0, (struct sockaddr *)host_addr, ctx->host_addr_len);
    if (bytes_sent == -1) {
        fprintf(stderr, "error: failed to send probe: %s\n", strerror(errno));
        free_ctx(ctx);
        return 1;
    }

    ctx->active_probes[active_probes_idx] = probe;
    
    printf("bytes sent: %ld\n", bytes_sent);
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
    t_context *ctx = init_ctx(argc, argv);
    if (!ctx) {
        return 1;
    }

    ctx->send_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (ctx->send_sock_fd == -1) {
        fprintf(stderr, "error: failed to create send socket: %s\n", strerror(errno));
        free_ctx(ctx);
        return 1;
    }

    ctx->recv_sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (ctx->recv_sock_fd == -1) {
        fprintf(stderr, "error: failed to create recv socket: %s\n", strerror(errno));
        free_ctx(ctx);
        return 1;
    }

    struct sockaddr send_sock_addr;
    ft_memset(&send_sock_addr, 0, sizeof(send_sock_addr));

    send_sock_addr.sa_family = AF_INET;
    if (bind(ctx->send_sock_fd, (struct sockaddr *) &send_sock_addr, sizeof(send_sock_addr)) == -1) {
        fprintf(stderr, "error: failed to bind send socket: %s\n", strerror(errno));
        free_ctx(ctx);
        return 1;
    }

    if (setsockopt(ctx->send_sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
        fprintf(stderr, "error: failed to set send socket options: %s\n", strerror(errno));
        free_ctx(ctx);
        return 1;
    }

    unsigned int icmp_filter = ~0U;
    icmp_filter &= ~(1U << ICMP_DEST_UNREACH);
    icmp_filter &= ~(1U << ICMP_TIME_EXCEEDED);

    if (setsockopt(ctx->recv_sock_fd, SOL_RAW, ICMP_FILTER, &icmp_filter, sizeof(icmp_filter)) == -1) {
        fprintf(stderr, "error: failed to set recv socket options: %s\n", strerror(errno));
        free_ctx(ctx);
        return 1;
    }

    send_probe(ctx);

    fd_set read_fds;

    FD_ZERO(&read_fds);
    FD_SET(ctx->recv_sock_fd, &read_fds);
    
    struct timeval  tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    int nfds = select(ctx->recv_sock_fd + 1, &read_fds, NULL, NULL, &tv);
    if (nfds == -1) {
        fprintf(stderr, "error: select failed: %s\n", strerror(errno));
    } else if (nfds > 0) {
        if (FD_ISSET(ctx->recv_sock_fd, &read_fds)) {
            t_icmp *icmp = &ctx->icmp;
            ssize_t bytes_received = recvfrom(ctx->recv_sock_fd, icmp->reply, sizeof(icmp->reply), 0, NULL, NULL);

            printf("bytes_received: %d\n", bytes_received);
            if (bytes_received > 0) {
                icmp->reply_len = bytes_received;

                if (icmp->reply_len < sizeof(struct iphdr)) {
                    return 1;
                }

                icmp->ip_hdr = (struct iphdr *)icmp->reply;
                size_t actual_ip_len = icmp->ip_hdr->ihl * 4;
                
                struct in_addr router_addr = { icmp->ip_hdr->saddr };
                printf("%s\n", inet_ntoa(router_addr));

                if (icmp->reply_len < actual_ip_len + sizeof(struct icmphdr)) {
                    return 1;
                }

                icmp->hdr = (struct icmphdr *)(icmp->reply + actual_ip_len);
                icmp->reply_offset = actual_ip_len + sizeof(struct icmphdr);

                if (parse_icmp_payload(icmp) != 0) {
                    return 1;
                }

                printf("src=%d, dst=%d\n", ntohs(icmp->data.udp_hdr->source),  ntohs(icmp->data.udp_hdr->dest));
            }
        }
    }

    free_ctx(ctx);
    return 0;
}
