#ifndef FT_TRACEROUTE_H
# define FT_TRACEROUTE_H

# define _POSIX_C_SOURCE 200112L
# define ROOT_UID 0
# define ICMP_REPLY_MAX_LEN 576

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
# include <sys/time.h>

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

typedef struct s_socket {
    int fd;
    struct sockaddr_storage *addr;
    socklen_t addr_len;
} t_socket;

typedef struct s_icmp {
    struct iphdr ip_hdr;
    struct icmphdr hdr;

    struct {
        struct iphdr ip_hdr;
        struct udphdr udp_hdr;
    } data;
} t_icmp;

typedef struct s_probe {
    size_t ttl;
    uint16_t dst_port;
    struct timeval send_time;
} t_probe;

typedef struct s_query {
    bool timeout;
    double rtt;
    
    t_probe *req;
    t_icmp *rep;
} t_query;

typedef struct s_hop {
    t_query *queries;
} t_hop;

typedef struct s_context {    
    t_option *options;
    struct sockaddr_storage *host_addr;
    socklen_t host_addr_len;

    uint16_t port;
    uint16_t current_port;
    size_t current_ttl;
    size_t max_ttl;
    size_t queries;
    size_t sim_queries;
    size_t packet_len;

    t_socket *udp_sock;
    t_socket *icmp_sock;

    t_probe **active_probes;
    t_hop *hops;
    size_t next_hop;
} t_context;

int parse_args(t_context *ctx, int argc, char **argv);
void print_helper(t_option *options);
t_socket *init_udp_socket();
t_socket *init_icmp_socket();
void free_ctx(t_context *ctx);
void free_socket(t_socket *sock);
void process_icmp_reply(t_context *ctx);

#endif
