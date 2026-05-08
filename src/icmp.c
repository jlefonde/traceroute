#include "traceroute.h"

static t_icmp *init_icmp(uint8_t *icmp_raw, size_t icmp_raw_size) {
    t_icmp *icmp = malloc(sizeof(t_icmp));
    if (!icmp) return NULL;

    if (icmp_raw_size < (sizeof(struct iphdr) + sizeof(struct icmphdr))) {
        free(icmp);
        return NULL;
    }

    ft_memcpy(&icmp->ip_hdr, icmp_raw, sizeof(struct iphdr));
    size_t outer_ip_len = icmp->ip_hdr.ihl * 4;

    size_t data_offset = outer_ip_len;
    ft_memcpy(&icmp->hdr, icmp_raw + outer_ip_len, sizeof(struct icmphdr));

    data_offset += sizeof(struct icmphdr);
    if (icmp_raw_size < (data_offset + sizeof(struct iphdr) + sizeof(struct udphdr))) {
        free(icmp);
        return NULL;
    }

    ft_memcpy(&icmp->data.ip_hdr, icmp_raw + data_offset, sizeof(struct iphdr));
    size_t inner_ip_len = icmp->data.ip_hdr.ihl * 4;

    data_offset += inner_ip_len;
    ft_memcpy(&icmp->data.udp_hdr, icmp_raw + data_offset, sizeof(struct udphdr));

    return icmp;
}

void process_icmp_reply(t_context *ctx) {
    uint8_t icmp_raw[ICMP_REPLY_MAX_LEN];
    ssize_t bytes_received = recvfrom(ctx->icmp_sock->fd , icmp_raw, sizeof(icmp_raw), 0, NULL, NULL);

    if (bytes_received > 0) {
        t_icmp *icmp = init_icmp(icmp_raw, (size_t)bytes_received);
        if (!icmp) {
            return;
        }

        struct in_addr router_addr = { icmp->ip_hdr.saddr };
        printf("router: %s, bytes_received: %ld\n", inet_ntoa(router_addr), bytes_received);

        size_t probe_id = ntohs(icmp->data.udp_hdr.dest) - ctx->port;
        size_t probe_ttl = probe_id / ctx->queries + 1;
        t_hop *hop = &ctx->hops[probe_ttl - 1];
        if (!hop) {
            return;
        }
        
        hop->ttl = probe_ttl;
        t_query *query = &hop->queries[(probe_id - 1) % ctx->queries];
        if (!query) {
            return;
        }

        query->rep = icmp;
        // query->req->is_active = false;

        printf("src=%d, dst=%d, ttl=%ld, idx=%ld\n", ntohs(icmp->data.udp_hdr.source), ntohs(icmp->data.udp_hdr.dest), hop->ttl, probe_id % 3);
    }
}