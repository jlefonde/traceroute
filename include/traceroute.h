#ifndef FT_TRACEROUTE_H
# define FT_TRACEROUTE_H

# define _POSIX_C_SOURCE 200112L

# include "../libft/include/libft.h"
# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <stdlib.h>
# include <stdbool.h>
# include <errno.h>
# include <arpa/inet.h>
# include <sys/select.h>
# include <netinet/in.h>
# include <netinet/ip.h>
# include <netinet/udp.h>
# include <netinet/ip_icmp.h>

# define ICMP_FILTER 1

typedef struct s_option {
    const char short_opt;
    const char *long_opt;
    const char *description;
    const bool has_argument;

    union {
        struct {
            const char *meta;
            char *value;
        } arg;

        struct {
            bool is_set;
        } flag;
    } data;
} t_option;

typedef struct s_icmp {
    uint8_t reply[1024];
    size_t reply_len;
    size_t reply_offset;

    struct iphdr *ip_hdr;
    struct icmphdr *hdr;

    struct {
        struct iphdr *ip_hdr;
        struct udphdr *udp_hdr;
    } data;
} t_icmp;

typedef struct s_context {
    struct sockaddr_storage *host_addr;
    socklen_t host_addr_len;
    t_option *options;

    uint16_t port;
    uint16_t current_port;
    size_t current_ttl;
    size_t max_ttl;
    size_t queries;
    size_t sim_queries;
    size_t packet_len;

    int send_sock_fd;
    int recv_sock_fd;
    t_icmp icmp;
} t_context;

#endif
