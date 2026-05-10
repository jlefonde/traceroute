#include "traceroute.h"

static t_icmp *init_icmp(uint8_t *icmp_raw, size_t icmp_raw_size) {
    t_icmp *icmp = malloc(sizeof(t_icmp));
    if (!icmp) {
        return NULL;
    }

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

double get_elapsed_ms(struct timeval start, struct timeval end) {
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;

    return (seconds * 1000.0) + (microseconds / 1000.0);
}

void process_icmp_reply(t_context *ctx) {
    uint8_t icmp_raw[ICMP_REPLY_MAX_LEN];

    ssize_t bytes_received = recvfrom(ctx->icmp_sock->fd , icmp_raw, sizeof(icmp_raw), 0, NULL, NULL);
    struct timeval recv_time;
    if (gettimeofday(&recv_time, NULL) == -1) {
        return;
    }

    if (bytes_received > 0) {
        t_icmp *icmp = init_icmp(icmp_raw, (size_t)bytes_received);
        if (!icmp) {
            return;
        }

        struct sockaddr_in *udp_addr = (struct sockaddr_in *)ctx->udp_sock->addr;
        if (udp_addr->sin_port != icmp->data.udp_hdr.source) {
            free(icmp);
            return;
        }

        size_t probe_dst_port = ntohs(icmp->data.udp_hdr.dest);
        if (probe_dst_port < ctx->port || probe_dst_port >= ctx->port + (ctx->max_ttl * ctx->queries)) {
            free(icmp);
            return;
        }

        size_t probe_id = probe_dst_port - ctx->port;
        size_t probe_ttl = (probe_id / ctx->queries) + 1;
        t_query *query = &ctx->hops[probe_ttl - 1].queries[probe_id % ctx->queries];
        if (!query->req || query->rep) {
            free(icmp);
            return;
        }

        query->rep = icmp;
        query->rtt = get_elapsed_ms(query->req->send_time, recv_time);

        if (!ctx->unreached_port_ttl && query->rep->hdr.type == ICMP_DEST_UNREACH && query->rep->hdr.code == ICMP_PORT_UNREACH) {
            ctx->unreached_port_ttl = probe_ttl;
        }
    }
}
