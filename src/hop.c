#include "traceroute.h"

int send_probe(t_context *ctx) {
    if (ctx->current_ttl > ctx->max_ttl) {
        return 0;
    }

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

    ctx->current_port++;
    if (hop_query_idx == ctx->queries - 1) {
        ctx->current_ttl++;
    }

    struct sockaddr_in *host_addr = (struct sockaddr_in *)ctx->host_addr;
    host_addr->sin_port = htons(probe->dst_port);

    if (setsockopt(ctx->udp_sock->fd , IPPROTO_IP, IP_TTL, &probe->ttl, sizeof(probe->ttl)) == -1) {
        return -1;
    }

    size_t hdr_len = sizeof(struct iphdr) + sizeof(struct udphdr);
    size_t payload_len = (ctx->packet_len > hdr_len) ? ctx->packet_len - hdr_len : 0;
    uint8_t *payload = ft_calloc(payload_len, sizeof(uint8_t));
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

    free(payload);
    return 1;
}

void update_active_queries(t_context *ctx) {
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

static void print_hop(t_context *ctx, t_hop *hop) {
    printf("%2zu", ctx->next_hop + 1);

    uint32_t last_addr = 0;
    for (size_t i = 0; i < ctx->queries; i++) {
        t_query *query = &hop->queries[i];

        if (query->rtt == -1) {
            printf("  *");
            continue;
        }

        uint32_t curr_addr = query->rep->ip_hdr.saddr;
        if (curr_addr != last_addr) {
            struct in_addr addr = { curr_addr };
            printf("  %s", inet_ntoa(addr));
            last_addr = curr_addr;
        }

        printf("  %.3f ms", query->rtt);
        if (query->rep->hdr.type == ICMP_DEST_UNREACH) {
            switch (query->rep->hdr.code) {
                case ICMP_NET_UNREACH: printf(" !N"); break;
                case ICMP_HOST_UNREACH: printf(" !H"); break;
                case ICMP_PROT_UNREACH: printf(" !P"); break;
                case ICMP_FRAG_NEEDED: printf(" !F"); break;
                case ICMP_SR_FAILED: printf(" !S"); break;
                case ICMP_PKT_FILTERED: printf(" !X"); break;
                case ICMP_PREC_VIOLATION: printf(" !V"); break;
                case ICMP_PREC_CUTOFF: printf(" !C"); break;
                case ICMP_PORT_UNREACH: break;
                default: printf(" !%d", query->rep->hdr.code);
            }
        }
    }

    printf("\n");
}

void print_available_hops(t_context *ctx) {
    while (ctx->next_hop < ctx->max_ttl && 
            (ctx->unreached_port_ttl == 0 || ctx->next_hop < ctx->unreached_port_ttl) &&
            !ctx->unreachable_hop) 
    {
        t_hop *hop = &ctx->hops[ctx->next_hop];
        bool hop_ready = true;
        size_t unreachable_count = 0;
        size_t timeout_count = 0;

        for (size_t i = 0; i < ctx->queries; i++) {
            t_query *query = &hop->queries[i];
            if (!query->req) {
                hop_ready = false;
                break;
            }

            if (!query->rep && query->rtt == 0) {
                hop_ready = false;
                break;
            }

            if (query->rep && query->rep->hdr.type == ICMP_DEST_UNREACH && query->rep->hdr.code != ICMP_PORT_UNREACH) {
                unreachable_count++;
            } else if (query->rtt == -1) {
                timeout_count++;
            }
        }

        if (!hop_ready) {
            break;
        }

        ctx->unreachable_hop = unreachable_count > 0 && (unreachable_count + timeout_count == ctx->queries);

        print_hop(ctx, hop);
        ctx->next_hop++;
    }
}

void set_timeout(t_context *ctx, struct timeval *timeout) {
    struct timeval *oldest = NULL;
    for (size_t i = 0; i < ctx->sim_queries; i++) {
        if (!ctx->active_queries[i]) {
            continue;
        }

        struct timeval *curr = &ctx->active_queries[i]->req->send_time;
        if (!oldest || curr->tv_sec < oldest->tv_sec ||
                (curr->tv_sec == oldest->tv_sec && curr->tv_usec < oldest->tv_usec)) {
            oldest = curr;
        }
    }

    if (!oldest) {
        timeout->tv_sec = 0;
        timeout->tv_usec = 10000;
        return;
    }

    struct timeval now;
    long sec, usec;
    gettimeofday(&now, NULL);
    sec = oldest->tv_sec + (long)ctx->timeout - now.tv_sec;
    usec = oldest->tv_usec - now.tv_usec;

    if (usec < 0) {
        sec--;
        usec += 1000000;
    }

    if (sec < 0) {
        timeout->tv_sec = 0;
        timeout->tv_usec = 0;
    } else {
        timeout->tv_sec = sec;
        timeout->tv_usec = usec;
    }
}
