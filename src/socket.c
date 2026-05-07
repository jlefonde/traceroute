#include "traceroute.h"

t_socket *init_udp_socket() {
    t_socket *udp_sock = malloc(sizeof(t_socket));
    if (!udp_sock) {
        fprintf(stderr, "error: failed to allocate UDP socket: %s\n", strerror(errno));
        return NULL;
    }

    udp_sock->fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock->fd  == -1) {
        fprintf(stderr, "error: failed to create UDP socket: %s\n", strerror(errno));
        free(udp_sock);
        return NULL;
    }

    udp_sock->addr_len = sizeof(struct sockaddr_storage);
    udp_sock->addr = malloc(udp_sock->addr_len);
    if (!udp_sock->addr) {
        fprintf(stderr, "error: failed to allocate UDP socket addr: %s\n", strerror(errno));
        free(udp_sock);
        return NULL;
    }

    ft_memset(udp_sock->addr, 0, udp_sock->addr_len);
    udp_sock->addr->ss_family = AF_INET;
    if (bind(udp_sock->fd , (struct sockaddr *)udp_sock->addr, udp_sock->addr_len) == -1) {
        fprintf(stderr, "error: failed to bind UDP socket: %s\n", strerror(errno));
        free(udp_sock);
        return NULL;
    }

    if (setsockopt(udp_sock->fd , SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
        fprintf(stderr, "error: failed to set UDP socket options: %s\n", strerror(errno));
        free(udp_sock);
        return NULL;
    }

    return udp_sock;
}

t_socket *init_icmp_socket() {
    t_socket *icmp_sock = malloc(sizeof(t_socket));
    if (!icmp_sock) {
        fprintf(stderr, "error: failed to allocate ICMP socket: %s\n", strerror(errno));
        return NULL;
    }

    icmp_sock->fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (icmp_sock->fd  == -1) {
        fprintf(stderr, "error: failed to create ICMP socket: %s\n", strerror(errno));
        free(icmp_sock);
        return NULL;
    }

    unsigned int icmp_filter = ~0U;
    icmp_filter &= ~(1U << ICMP_DEST_UNREACH);
    icmp_filter &= ~(1U << ICMP_TIME_EXCEEDED);

    if (setsockopt(icmp_sock->fd , SOL_RAW, ICMP_FILTER, &icmp_filter, sizeof(icmp_filter)) == -1) {
        fprintf(stderr, "error: failed to set ICMP socket options: %s\n", strerror(errno));
        free(icmp_sock);
        return NULL;
    }

    return icmp_sock;
}
