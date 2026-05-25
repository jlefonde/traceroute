#include "traceroute.h"

#define MIN_TTL_VALUE 1
#define MAX_TTL_VALUE 255

static bool parse_int_arg(t_option *option, int min_val, int max_val, void *out, size_t out_size) {
    if (!option->value) {
        return true;
    }

    int val = ft_atoi(option->value);
    if (val < min_val || val > max_val) {
        fprintf(stderr, "error: %s out of range [%d,%d]\n", option->meta, min_val, max_val);
        return false;
    }

    if (out_size == sizeof(uint8_t)) {
        *(uint8_t *)out = (uint8_t)val;
    } else if (out_size == sizeof(uint16_t)) {
        *(uint16_t *)out = (uint16_t)val;
    } else if (out_size == sizeof(uint32_t)) {
        *(uint32_t *)out = (uint32_t)val;
    } else if (out_size == sizeof(uint64_t)) {
        *(uint64_t *)out = (uint64_t)val;
    }

    return true;
}

static bool set_host_addr(t_context *ctx) {
    struct addrinfo *res;
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM,
        .ai_flags = AI_ADDRCONFIG,
    };

    int err_code = getaddrinfo(ctx->host_str, NULL, &hints, &res);
    if (err_code != 0) {
        fprintf(stderr, "error: '%s': %s\n", ctx->host_str, gai_strerror(err_code));
        return false;
    }

    ctx->host_addr_len = res->ai_addrlen;
    ctx->host_addr = malloc(ctx->host_addr_len);
    if (!ctx->host_addr) {
        fprintf(stderr, "error: failed to allocate host address: %s\n", strerror(errno));
        freeaddrinfo(res);
        return false;
    }

    ft_memcpy(ctx->host_addr, res->ai_addr, ctx->host_addr_len);

    freeaddrinfo(res);
    return true;
}

t_context *init_ctx(t_option *options, char *host) {
    t_context *ctx = ft_calloc(1, sizeof(t_context));
    if (!ctx) {
        fprintf(stderr, "error: failed to init context: %s\n", strerror(errno));
        return NULL;
    }

    ctx->pid = getpid();
    ctx->current_seq = 1;
    ctx->max_ttl = 30;
    ctx->first_ttl = 1;
    ctx->queries = 3;
    ctx->sim_queries = 16;
    ctx->no_reverse = false;
    ctx->method = MTD_DEFAULT;

    ctx->host_str = host;
    ctx->port = 33434;
    ctx->current_port = ctx->port;
    ctx->timeout = 5;
    ctx->packet_len = 60;
    ctx->unreached_port_ttl = 0;

    t_option *no_rev_opt = get_option_by_index(options, OPT_NO_REVERSE);
    if (no_rev_opt && no_rev_opt->is_set) {
        ctx->no_reverse = true;
    }

    if (!parse_int_arg(get_option_by_index(options, OPT_MAX_TTL), MIN_TTL_VALUE, MAX_TTL_VALUE, &ctx->max_ttl, sizeof(ctx->max_ttl))) {
        free_ctx(ctx);
        return NULL;
    }

    if (!parse_int_arg(get_option_by_index(options, OPT_FIRST_TTL), MIN_TTL_VALUE, MAX_TTL_VALUE, &ctx->first_ttl, sizeof(ctx->first_ttl))) {
        free_ctx(ctx);
        return NULL;
    }

    if (!parse_int_arg(get_option_by_index(options, OPT_QUERIES), 1, 10, &ctx->queries, sizeof(ctx->queries))) {
        free_ctx(ctx);
        return NULL;
    }

    if (!parse_int_arg(get_option_by_index(options, OPT_SIM_QUERIES), 1, 32, &ctx->sim_queries, sizeof(ctx->sim_queries))) {
        free_ctx(ctx);
        return NULL;
    }

    if (ctx->first_ttl > ctx->max_ttl) {
        fprintf(stderr, "error: first_ttl out of range\n");
        free_ctx(ctx);
        return NULL;
    }

    ctx->current_ttl = ctx->first_ttl;
    ctx->next_hop = ctx->first_ttl - 1;

    t_option *icmp = get_option_by_index(options, OPT_ICMP);
    if (icmp && icmp->is_set) {
        ctx->method = MTD_ICMP;
    }

    if (!set_host_addr(ctx)) {
        free_ctx(ctx);
        return NULL;
    }

    if (ctx->method == MTD_DEFAULT) {
        ctx->udp_sock = init_udp_socket();
        if (!ctx->udp_sock) {
            free_ctx(ctx);
            return NULL;
        }
    }

    ctx->icmp_sock = init_icmp_socket();
    if (!ctx->icmp_sock) {
        free_ctx(ctx);
        return NULL;
    }

    ctx->active_queries = ft_calloc(ctx->sim_queries, sizeof(t_query *));
    if (!ctx->active_queries) {
        fprintf(stderr, "error: failed to allocate active queries: %s\n", strerror(errno));
        free_ctx(ctx);
        return NULL;
    }

    ctx->hops = ft_calloc(ctx->max_ttl, sizeof(t_hop));
    if (!ctx->hops) {
        fprintf(stderr, "error: failed to allocate hops: %s\n", strerror(errno));
        free_ctx(ctx);
        return NULL;
    }

    for (size_t i = 0; i < ctx->max_ttl; i++) {
        ctx->hops[i].queries = ft_calloc(ctx->queries, sizeof(t_query));
        if (!ctx->hops[i].queries) {
            fprintf(stderr, "error: failed to allocate hops' queries: %s\n", strerror(errno));
            free_ctx(ctx);
            return NULL;
        }
    }

    return ctx;
}

void free_hops(t_context *ctx) {
    for (size_t i = 0; i < ctx->max_ttl; i++) {
        if (!ctx->hops[i].queries) {
            continue;
        }

        for (size_t j = 0; j < ctx->queries; j++) {
            free(ctx->hops[i].queries[j].rep);

            t_probe *req = ctx->hops[i].queries[j].req;
            if (!req) {
                continue;
            }
            
            switch (ctx->method) {
                case MTD_ICMP: free(req->icmp.payload); break;
                default: free(req->udp.payload);
            }

            free(req);
        }

        free(ctx->hops[i].queries);
    }

    free(ctx->hops);
}

void free_ctx(t_context *ctx) {
    free(ctx->host_addr);
    free(ctx->active_queries);

    free_socket(ctx->udp_sock);
    free_socket(ctx->icmp_sock);

    if (ctx->hops) {
        // free_hops(ctx);
    }

    free(ctx);
}
