#include "traceroute.h"

static int set_host_addr(t_context *ctx) {
    struct addrinfo *res;
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM,
        .ai_flags = AI_ADDRCONFIG,
    };

    int err_code = getaddrinfo(ctx->host_str, NULL, &hints, &res);
    if (err_code != 0) {
        fprintf(stderr, "error: '%s': %s\n", ctx->host_str, gai_strerror(err_code));
        return -1;
    }

    ctx->host_addr_len = res->ai_addrlen;
    ctx->host_addr = malloc(ctx->host_addr_len);
    if (!ctx->host_addr) {
        fprintf(stderr, "error: failed to allocate host address: %s\n", strerror(errno));
        freeaddrinfo(res);
        return -1;
    }

    ft_memcpy(ctx->host_addr, res->ai_addr, ctx->host_addr_len);

    freeaddrinfo(res);
    return 0;
}

t_context *init_ctx(t_option *options, char *host) {
    t_context *ctx = ft_calloc(1, sizeof(t_context));
    if (!ctx) {
        fprintf(stderr, "error: failed to init context: %s\n", strerror(errno));
        return NULL;
    }

    ctx->options = options;
    ctx->host_str = host;
    ctx->port = 33434;
    ctx->current_port = ctx->port;
    ctx->current_ttl = 1;
    ctx->max_ttl = 30;
    ctx->queries = 3;
    ctx->sim_queries = 16;
    ctx->timeout = 5;
    ctx->packet_len = 60;
    ctx->unreached_port_ttl = 0;
    ctx->next_hop = 0;

    if (set_host_addr(ctx) != 0) {
        free_ctx(ctx);
        return NULL;
    }

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

void free_ctx(t_context *ctx) {
    free(ctx->host_addr);
    free(ctx->active_queries);

    free_socket(ctx->udp_sock);
    free_socket(ctx->icmp_sock);

    if (ctx->hops) {
        for (size_t i = 0; i < ctx->max_ttl; i++) {
            if (ctx->hops[i].queries) {
                for (size_t j = 0; j < ctx->queries; j++) {
                    free(ctx->hops[i].queries[j].req);
                    free(ctx->hops[i].queries[j].rep);
                }
                free(ctx->hops[i].queries);
            }
        }

        free(ctx->hops);
    }

    free(ctx);
}
