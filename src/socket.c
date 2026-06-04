#include "../include/traceroute.h"

void free_socket(t_socket *sock) {
    if (!sock) {
        return;
    }

    if (sock->fd != -1) {
        close(sock->fd);
    }

    free(sock->addr);
    free(sock);
}

t_socket *init_udp_socket() {
    t_socket *udp_sock = malloc(sizeof(t_socket));
    if (!udp_sock) {
        fprintf(stderr, "error: failed to allocate UDP socket: %s\n", strerror(errno));
        return NULL;
    }

    udp_sock->addr = NULL;
    udp_sock->fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock->fd  == -1) {
        fprintf(stderr, "error: failed to create UDP socket: %s\n", strerror(errno));
        free_socket(udp_sock);
        return NULL;
    }

    udp_sock->addr = malloc(sizeof(struct sockaddr_storage));
    if (!udp_sock->addr) {
        free_socket(udp_sock);
        return NULL;
    }

    ft_memset(udp_sock->addr, 0, sizeof(struct sockaddr_storage));

    struct sockaddr_in *addr4 = (struct sockaddr_in *)udp_sock->addr;
    addr4->sin_family = AF_INET;
    addr4->sin_port = htons((getpid() & 0xFFFF) | 0x8000); 
    if (bind(udp_sock->fd, (struct sockaddr *)addr4, sizeof(struct sockaddr_in)) == -1) {
        free_socket(udp_sock);
        return NULL;
    }

    if (setsockopt(udp_sock->fd , SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
        fprintf(stderr, "error: failed to set UDP socket options: %s\n", strerror(errno));
        free_socket(udp_sock);
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

    icmp_sock->addr = NULL;
    icmp_sock->fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (icmp_sock->fd  == -1) {
        fprintf(stderr, "error: failed to create ICMP socket: %s\n", strerror(errno));
        free(icmp_sock);
        return NULL;
    }

    unsigned int icmp_filter = ~0U;
    icmp_filter &= ~(1U << ICMP_DEST_UNREACH);
    icmp_filter &= ~(1U << ICMP_TIME_EXCEEDED);
    icmp_filter &= ~(1U << ICMP_ECHOREPLY);

    if (setsockopt(icmp_sock->fd , SOL_RAW, ICMP_FILTER, &icmp_filter, sizeof(icmp_filter)) == -1) {
        fprintf(stderr, "error: failed to set ICMP socket options: %s\n", strerror(errno));
        free(icmp_sock);
        return NULL;
    }

    return icmp_sock;
}
