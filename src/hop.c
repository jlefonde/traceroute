#include "traceroute.h"

static int get_active_query_idx(t_context *ctx) {
    for (size_t i = 0; i < ctx->sim_queries; i++) {
        if (ctx->active_queries[i] == NULL) {
            return i;
        }
    }

    return -1;
}

static int get_hop_query_idx(t_context *ctx) {
    switch (ctx->method) {
        case MTD_ICMP:
            break;
        default:
            return (ctx->current_port - ctx->port) % ctx->queries;
    }

    return -1;
}

static t_probe *init_udp_probe(t_context *ctx, uint8_t *payload, size_t payload_len) {
    t_probe *probe = malloc(sizeof(t_probe));
    if (!probe) {
        free(payload);
        return NULL; 
    }

    probe->send_sock_fd = ctx->udp_sock->fd;
    probe->ttl = ctx->current_ttl;
    probe->udp.dst_port = ctx->current_port++;
    probe->udp.payload = payload;
    probe->udp.payload_len = payload_len;

    return probe;
}

static t_probe *init_icmp_probe(t_context *ctx, uint8_t *payload, size_t payload_len) {
    t_probe *probe = malloc(sizeof(t_probe));
    if (!probe) {
        free(payload);
        return NULL; 
    }

    probe->send_sock_fd = ctx->icmp_sock->fd;
    probe->ttl = ctx->current_ttl;
    probe->icmp.hdr.type = ICMP_ECHO;
    probe->icmp.hdr.code = 0;
    probe->icmp.hdr.un.echo.id = ntohs(ctx->pid);
    probe->icmp.hdr.un.echo.sequence = ntohs(ctx->current_seq++);
    probe->icmp.hdr.checksum = 0;
    probe->icmp.payload = payload;
    probe->icmp.payload_len = payload_len;

    set_icmp_checksum(probe);

    return probe;
}

static t_probe *init_probe(t_context *ctx) {
    size_t hdr_len = sizeof(struct iphdr) + sizeof(struct udphdr);
    size_t payload_len = (ctx->packet_len > hdr_len) ? ctx->packet_len - hdr_len : 0;
    uint8_t *payload = malloc(sizeof(uint8_t) * payload_len);
    if (!payload) {
        return NULL;
    }

    for (size_t i = 0; i < payload_len; i++) {
        payload[i] = '@' + i;
    }

    switch (ctx->method) {
        case MTD_ICMP:
            return init_icmp_probe(ctx, payload, payload_len);
        default:
            return init_udp_probe(ctx, payload, payload_len);
    }

    return NULL;
}

int send_icmp_probe(t_context *ctx, t_probe *probe) {
    size_t buffer_len = sizeof(probe->icmp.hdr) + probe->icmp.payload_len;
    uint8_t *buffer = malloc(buffer_len);
    if (!buffer) {
        return -1;
    }

    ft_memcpy(buffer, &probe->icmp.hdr, sizeof(probe->icmp.hdr));
    ft_memcpy(buffer + sizeof(probe->icmp.hdr), probe->icmp.payload, probe->icmp.payload_len);

    struct sockaddr_in *host_addr = (struct sockaddr_in *)ctx->host_addr;

    if (gettimeofday(&probe->send_time, NULL) == -1) {
        free(buffer);
        return -1;
    }

    ssize_t bytes_sent = sendto(probe->send_sock_fd, buffer, buffer_len, 0, (struct sockaddr *)host_addr, ctx->host_addr_len);
    free(buffer);
    if (bytes_sent == -1) {
        return -1;
    }

    return 1;
}

int send_udp_probe(t_context *ctx, t_probe *probe) {
    struct sockaddr_in *host_addr = (struct sockaddr_in *)ctx->host_addr;
    host_addr->sin_port = htons(probe->udp.dst_port);

    if (gettimeofday(&probe->send_time, NULL) == -1) {
        return -1;
    }

    ssize_t bytes_sent = sendto(probe->send_sock_fd, probe->udp.payload, probe->udp.payload_len, 0, (struct sockaddr *)host_addr, ctx->host_addr_len);
    if (bytes_sent == -1) {
        return -1;
    }

    return 1;
}

int send_probe(t_context *ctx) {
    if (ctx->current_ttl > ctx->max_ttl) {
        return 0;
    }

    if (ctx->method == MTD_DEFAULT && ctx->unreached_port_ttl > 0 && ctx->current_ttl > ctx->unreached_port_ttl) {
        return 0;
    }

    int active_query_idx = get_active_query_idx(ctx);
    if (active_query_idx == -1) {
        return 0;
    }

    size_t hop_query_idx = get_hop_query_idx(ctx);
    t_query *query = &ctx->hops[ctx->current_ttl - 1].queries[hop_query_idx];
    
    t_probe *probe = init_probe(ctx);
    if (!probe) {
        return -1;
    }

    query->req = probe;
    ctx->active_queries[active_query_idx] = query;
    
    if (hop_query_idx == ctx->queries - 1) {
        ctx->current_ttl++;
    }

    if (setsockopt(probe->send_sock_fd , IPPROTO_IP, IP_TTL, &probe->ttl, sizeof(probe->ttl)) == -1) {
        return -1;
    }

    switch (ctx->method) {
        case MTD_ICMP:
            return send_icmp_probe(ctx, probe);
        case MTD_DEFAULT:
        default:
            return send_udp_probe(ctx, probe);
    }
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

            if (ctx->no_reverse) {
                printf("  %s", inet_ntoa(addr));
            } else {
                struct sockaddr_in sock_addr = { 
                    .sin_addr = addr,
                    .sin_family = AF_INET,
                };

                char host[HOST_MAX_LEN];
                if (getnameinfo((struct sockaddr *)&sock_addr, sizeof(struct sockaddr_in), host, HOST_MAX_LEN, NULL, 0, NI_NAMEREQD) == 0) {
                    printf("  %s", host);
                } else {
                    printf("  %s", inet_ntoa(addr));
                }
            }

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
