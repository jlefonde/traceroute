#include "../include/traceroute.h"

static int parse_icmp_inner(t_icmp *icmp, uint8_t *icmp_raw, size_t offset, size_t max_size, 
                            int (*parse_inner_hdr)(t_icmp *, uint8_t *, size_t, size_t)) {
    if (max_size < offset + sizeof(struct iphdr)) {
        return -1;
    }

    ft_memcpy(&icmp->inner.ip_hdr, icmp_raw + offset, sizeof(struct iphdr));
    size_t inner_ip_len = icmp->inner.ip_hdr.ihl * 4;

    return parse_inner_hdr(icmp, icmp_raw, offset + inner_ip_len, max_size);
}

static int parse_icmp_inner_icmp(t_icmp *icmp, uint8_t *icmp_raw, size_t offset, size_t max_size) {
    if (max_size < offset + sizeof(struct icmphdr)) {
        return -1;
    }

    ft_memcpy(&icmp->inner.icmp_hdr, icmp_raw + offset, sizeof(struct icmphdr));

    return 0;
}

static int parse_icmp_inner_udp(t_icmp *icmp, uint8_t *icmp_raw, size_t offset, size_t max_size) {
    if (max_size < offset + sizeof(struct udphdr)) {
        return -1;
    }

    ft_memcpy(&icmp->inner.udp_hdr, icmp_raw + offset, sizeof(struct udphdr));

    return 0;
}

static t_icmp *init_icmp_reply(uint8_t *icmp_raw, size_t icmp_raw_size, 
                               int (*parse_inner_hdr)(t_icmp *, uint8_t *, size_t, size_t)) {
    t_icmp *icmp = malloc(sizeof(t_icmp));
    if (!icmp) return NULL;

    if (icmp_raw_size < sizeof(struct iphdr)) {
        free(icmp);
        return NULL;
    }

    ft_memcpy(&icmp->ip_hdr, icmp_raw, sizeof(struct iphdr));
    
    size_t offset = icmp->ip_hdr.ihl * 4;
    if (icmp_raw_size < offset + sizeof(struct icmphdr)) {
        free(icmp);
        return NULL;
    }

    ft_memcpy(&icmp->hdr, icmp_raw + offset, sizeof(struct icmphdr));
    offset += sizeof(struct icmphdr);

    if (parse_icmp_inner(icmp, icmp_raw, offset, icmp_raw_size, parse_inner_hdr) == -1) {
        free(icmp);
        return NULL;
    }

    return icmp;
}


static int get_icmp_probe_id(t_context *ctx, t_icmp *icmp) {
    if (icmp->inner.icmp_hdr.type != ICMP_ECHO && icmp->inner.icmp_hdr.type != ICMP_ECHOREPLY) {
        return -1;
    }

    if (icmp->inner.icmp_hdr.un.echo.id != htons(ctx->pid)) {
        return -1;
    }

    size_t seq = ntohs(icmp->inner.icmp_hdr.un.echo.sequence);
    if (seq == 0 || seq > (ctx->max_ttl * ctx->queries)) {
        return -1;
    }

    return seq - 1;
}

static int get_udp_probe_id(t_context *ctx, t_icmp *icmp) {
    struct sockaddr_in *udp_addr = (struct sockaddr_in *)ctx->udp_sock->addr;
    if (udp_addr->sin_port != icmp->inner.udp_hdr.source) {
        return -1;
    }

    size_t probe_dst_port = ntohs(icmp->inner.udp_hdr.dest);
    if (probe_dst_port < ctx->port || probe_dst_port >= ctx->port + (ctx->max_ttl * ctx->queries)) {
        return -1;
    }

    return (probe_dst_port - ctx->port);
}

static t_query *get_hop_query(t_context *ctx, t_icmp *icmp, size_t probe_id) {
    size_t probe_ttl = (probe_id / ctx->queries) + ctx->first_ttl;
    t_query *query = &ctx->hops[probe_ttl - 1].queries[probe_id % ctx->queries];
    if (!query->req || query->rep) {
        free(icmp);
        return NULL;
    }

    return query;
}

double get_elapsed_ms(struct timeval start, struct timeval end) {
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;

    return (seconds * 1000.0) + (microseconds / 1000.0);
}

void set_icmp_checksum(t_probe *probe) {
    uint32_t sum = 0;
    uint8_t *payload = probe->icmp.payload;
    size_t payload_len = probe->icmp.payload_len;

    sum += (probe->icmp.hdr.type << 8) + probe->icmp.hdr.code;

    sum += htons(probe->icmp.hdr.un.echo.id);
    sum += htons(probe->icmp.hdr.un.echo.sequence);

    for (size_t i = 0; i < payload_len; i += 2) {
        sum += (payload[i] << 8) | (i + 1 < payload_len ? payload[i + 1] : 0);
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    probe->icmp.hdr.checksum = htons(~sum);
}

void process_icmp_reply_icmp(t_context *ctx, uint8_t *icmp_raw, ssize_t bytes_received, struct timeval recv_time) {
    t_icmp *icmp = init_icmp_reply(icmp_raw, (size_t)bytes_received, parse_icmp_inner_icmp);
    if (!icmp) {
        return;
    }

    size_t probe_id = get_icmp_probe_id(ctx, icmp);
    if (probe_id == -1) {
        free(icmp);
        return;
    }

    t_query *query = get_hop_query(ctx, icmp, probe_id);
    if (!query) {
        free(icmp);
        return;
    }

    query->rep = icmp;
    query->rtt = get_elapsed_ms(query->req->send_time, recv_time);
}

void process_icmp_reply_udp(t_context *ctx, uint8_t *icmp_raw, ssize_t bytes_received, struct timeval recv_time) {
    t_icmp *icmp = init_icmp_reply(icmp_raw, (size_t)bytes_received, parse_icmp_inner_udp);
    if (!icmp) {
        return;
    }

    size_t probe_id = get_udp_probe_id(ctx, icmp);
    if (probe_id == -1) {
        free(icmp);
        return;
    }

    t_query *query = get_hop_query(ctx, icmp, probe_id);
    if (!query) {
        free(icmp);
        return;
    }

    query->rep = icmp;
    query->rtt = get_elapsed_ms(query->req->send_time, recv_time);

    if (!ctx->unreached_port_ttl && query->rep->hdr.type == ICMP_DEST_UNREACH && query->rep->hdr.code == ICMP_PORT_UNREACH) {            
        ctx->unreached_port_ttl = (probe_id / ctx->queries) + ctx->first_ttl;;
    }
}

void process_icmp_reply(t_context *ctx, uint8_t *icmp_raw, ssize_t bytes_received, struct timeval recv_time,
                        int (*parse_inner_hdr)(t_icmp *, uint8_t *, size_t, size_t),
                        int (*get_probe_id)(t_context *, t_icmp *)) 
{
    t_icmp *icmp = init_icmp_reply(icmp_raw, (size_t)bytes_received, parse_inner_hdr);
    if (!icmp) {
        return;
    }

    size_t probe_id = get_probe_id(ctx, icmp);
    if (probe_id == -1) {
        free(icmp);
        return;
    }

    t_query *query = get_hop_query(ctx, icmp, probe_id);
    if (!query) {
        free(icmp);
        return;
    }

    query->rep = icmp;
    query->rtt = get_elapsed_ms(query->req->send_time, recv_time);

    if (ctx->method == MTD_DEFAULT && !ctx->unreached_port_ttl && 
        query->rep->hdr.type == ICMP_DEST_UNREACH && query->rep->hdr.code == ICMP_PORT_UNREACH) {            
        ctx->unreached_port_ttl = (probe_id / ctx->queries) + ctx->first_ttl;
    }
}

void recv_icmp_reply(t_context *ctx) {
    uint8_t icmp_raw[ICMP_REPLY_MAX_LEN];

    ssize_t bytes_received = recvfrom(ctx->icmp_sock->fd , icmp_raw, sizeof(icmp_raw), 0, NULL, NULL);
    struct timeval recv_time;
    if (bytes_received <= 0 || gettimeofday(&recv_time, NULL) == -1) {
        return;
    }

    switch (ctx->method) {
        case MTD_ICMP:
            process_icmp_reply(ctx, icmp_raw, bytes_received, recv_time, parse_icmp_inner_icmp, get_icmp_probe_id);
            break;
        case MTD_DEFAULT:
        default:
            process_icmp_reply(ctx, icmp_raw, bytes_received, recv_time, parse_icmp_inner_udp, get_udp_probe_id);
    }
}
