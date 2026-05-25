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
# include <sys/time.h>

# define ROOT_UID 0
# define HELPER_SPACE_LEN 32
# define ICMP_FILTER 1
# define ICMP_REPLY_MAX_LEN 576
# define HOST_MAX_LEN 256

typedef enum e_option_idx {
    OPT_HELP,
    OPT_MAX_TTL,
    OPT_FIRST_TTL,
    OPT_QUERIES,
    OPT_SIM_QUERIES,
    OPT_NO_REVERSE,
    OPT_ICMP,
    OPT_COUNT, // Keep at the end of the enum
} t_option_idx;

typedef enum e_method {
    MTD_DEFAULT,
    MTD_ICMP,
} t_method;

typedef struct s_option {
    const char short_opt;
    const char *long_opt;
    const char *description;
    const bool has_argument;

    const char *meta;
    char *value;
    bool is_set;
} t_option;

typedef struct s_socket {
    int fd;
    struct sockaddr_storage *addr;
} t_socket;

typedef struct s_icmp {
    struct iphdr ip_hdr;
    struct icmphdr hdr;

    struct {
        struct iphdr ip_hdr;
        union {
            struct udphdr udp_hdr;
            struct icmphdr icmp_hdr;
        };
    } inner;

    uint8_t *payload;
    size_t payload_len;
} t_icmp;

typedef struct s_udp {
    uint16_t dst_port;
    uint8_t *payload;
    size_t payload_len;
} t_udp;

typedef struct s_probe {
    int ttl;
    struct timeval send_time;
    int send_sock_fd;

    union
    {
        t_udp udp;
        t_icmp icmp;
    };
} t_probe;

typedef struct s_query {
    double rtt;
    
    t_probe *req;
    t_icmp *rep;
} t_query;

typedef struct s_hop {
    t_query *queries;
} t_hop;

typedef struct s_context {    
    char *host_str;
    struct sockaddr_storage *host_addr;
    socklen_t host_addr_len;
    pid_t pid;

    int current_seq;
    int current_ttl;
    uint8_t max_ttl;
    uint8_t first_ttl;
    uint8_t queries;
    uint8_t sim_queries;
    bool no_reverse;
    t_method method;

    uint16_t port;
    uint16_t current_port;
    size_t packet_len;
    size_t timeout;

    t_socket *udp_sock;
    t_socket *icmp_sock;

    t_query **active_queries;
    t_hop *hops;
    size_t next_hop;
    size_t unreached_port_ttl;
    bool unreachable_hop;
} t_context;

t_option *init_options();
int parse_short_option(t_option *options, char ***argv);
int parse_long_option(t_option *options, char ***argv);
int parse_args(int argc, char **argv, t_option *options, char **host);
void print_helper(t_option *options);
t_option *get_option_by_index(t_option *options, t_option_idx idx);
char *get_option_value(t_option *options, t_option_idx idx);
t_socket *init_udp_socket();
t_socket *init_icmp_socket();
t_context *init_ctx(t_option *options, char *host);
void free_ctx(t_context *ctx);
void free_socket(t_socket *sock);
int send_probe(t_context *ctx);
void set_timeout(t_context *ctx, struct timeval *timeout);
void update_active_queries(t_context *ctx);
void print_available_hops(t_context *ctx);
void process_icmp_reply(t_context *ctx);
double get_elapsed_ms(struct timeval start, struct timeval end);
t_option *get_option_short(t_option *options, char opt);
t_option *get_option_long(t_option *options, char *opt);
void set_icmp_checksum(t_probe *probe);

#endif
