#define _POSIX_C_SOURCE 200112L

#include "../libft/include/libft.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/select.h>

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

typedef struct s_context {
    struct sockaddr_storage *host_addr;
    socklen_t host_addr_len;
    t_option *options;
} t_context;

void free_ctx(t_context *ctx) {
    free(ctx->host_addr);
    free(ctx);
}

static t_option *get_option_short(t_option *options, char opt) {
    for (size_t i = 0; options[i].short_opt; i++) {
        if (options[i].short_opt == opt) {
            return &options[i];
        }
    }

    return NULL;
}

static t_option *get_option_long(t_option *options, char *opt) {
    for (size_t i = 0; options[i].long_opt; i++) {
        if (options[i].long_opt[0] && ft_strcmp(options[i].long_opt, opt) == 0) {
            return &options[i];
        }
    }

    return NULL;
}

static void print_spaces(int count) {
    while (count-- > 0) {
        ft_printf(" ");
    }
}

static void print_helper(t_option *options) {
    ft_printf("usage: ./ft_traceroute <host> [options]\n");
    ft_printf("options:\n");

    for (size_t i = 0; options[i].short_opt || (options[i].long_opt && options[i].long_opt[0]); i++) {
        int len = 0;
        
        ft_printf("  ");
        len += 2;
        
        if (options[i].short_opt) {
            ft_printf("-%c", options[i].short_opt);
            len += 2;
            if (options[i].long_opt && options[i].long_opt[0]) {
                ft_printf("  ");
                len += 2;
            }
        } else if (options[i].long_opt && options[i].long_opt[0]) {
            ft_printf("    ");
            len += 4;
        }

        if (options[i].long_opt && options[i].long_opt[0]) {
            ft_printf("--%s", options[i].long_opt);
            len += 2 + ft_strlen(options[i].long_opt);
        }

        if (options[i].has_argument && options[i].data.arg.meta) {
            ft_printf(" <%s>", options[i].data.arg.meta);
            len += 3 + ft_strlen(options[i].data.arg.meta);
        }

        int pad = 24 - len;
        if (pad > 0) {
            print_spaces(pad);
        } else {
            ft_printf(" ");
        }

        ft_printf("%s\n", options[i].description ? options[i].description : "");
    }
}

static bool check_helper(char **argv) {
    for (char **arg = argv + 1; *arg; arg++) {
        if ((*arg)[0] != '-') {
            continue;
        }

        if ((*arg)[1] != '-' && ft_strchr(*arg + 1, 'h') != NULL) {
            return true;
        } else if (ft_strcmp(*arg + 2, "help") == 0) {
            return true;
        }
    }

    return false;
}

static int parse_short_option(t_context *ctx, char ***argv) {
    char *curr_str = **argv;
    int len = ft_strlen(curr_str);

    for (int i = 1; i < len; i++) {
        int option_idx = -1;
        char short_opt = curr_str[i];

        t_option *option = get_option_short(ctx->options, short_opt);
        if (!option) {
            ft_fprintf(STDERR_FILENO, "error: bad option '-%c'\n", short_opt);
            return 1;
        }

        if (!option->has_argument) {
            option->data.flag.is_set = true;
            continue;
        } 

        if (i == len - 1 && (*argv)[1]) {
            (*argv)++;
            option->data.arg.value = **argv;
        }
        else {
            ft_fprintf(STDERR_FILENO, "error: option '-%c' requires an argument\n", short_opt);
            return 1;
        }
    }

    return 0;
}

static int parse_long_option(t_context *ctx, char ***argv) {
    int option_idx = -1;
    char *long_opt = (**argv) + 2;

    t_option *option = get_option_long(ctx->options, long_opt);
    if (!option) {
        ft_fprintf(STDERR_FILENO, "error: bad option '--%s'\n", long_opt);
        return 1;
    }

    if (!option->has_argument) {
        option->data.flag.is_set = true;
    }
    else if ((*argv)[1]) {
        (*argv)++;
        option->data.arg.value = **argv;
    }
    else {
        ft_fprintf(STDERR_FILENO, "error: option '--%s' requires an argument\n", long_opt);
        return 1;
    }

    return 0;
}

static int set_host_address(t_context *ctx, char *host_str) {
    if (!host_str) {
        ft_fprintf(STDERR_FILENO, "error: missing \"host\" argument\n");
        return 1;
    }

    struct addrinfo *res;
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM,
        .ai_flags = AI_ADDRCONFIG,
    };

    int err_code = getaddrinfo(host_str, NULL, &hints, &res);
    if (err_code != 0) {
        ft_fprintf(STDERR_FILENO, "error: '%s': %s\n", host_str, gai_strerror(err_code));
        return 1;
    }

    ctx->host_addr_len = res->ai_addrlen;
    ctx->host_addr = malloc(ctx->host_addr_len);
    if (!ctx->host_addr) {
        ft_fprintf(STDERR_FILENO, "error: failed to allocate host address: %s\n", strerror(errno));
        freeaddrinfo(res);
        return 1;
    }

    ft_memcpy(ctx->host_addr, res->ai_addr, ctx->host_addr_len);
    
    freeaddrinfo(res);
    return 0;
}

static int parse_args(t_context *ctx, int argc, char **argv) {
    if (check_helper(argv)) {
        return 2;
    }

    char *host_str = NULL;
    for (char **arg = argv + 1; *arg; arg++) {
        int err = 0;

        if ((*arg)[0] == '-' && (*arg)[1] && (*arg)[1] != '-') {
            err = parse_short_option(ctx, &arg);
        }
        else if ((*arg)[0] == '-' && (*arg)[1] == '-' && (*arg)[2] && (*arg)[2] != '-') {
            err = parse_long_option(ctx, &arg);
        }
        else if (!host_str) {
            host_str = *arg;
        } else {
            ft_fprintf(STDERR_FILENO, "error: extra argument '%s'\n", *arg);
            return 1;
        }

        if (err != 0) {
            return 1;
        }
    }
    
    if (set_host_address(ctx, host_str) != 0) {
        return 1;
    }

    return 0;
}

static t_option *init_options() {
    static t_option options[] = {
        {
            .short_opt = 'h',
            .long_opt = "help",
            .description = "Print this help message and exit",
            .has_argument = false,
        },
    };

    return options;
}

t_context *init_ctx(int argc, char **argv) {
    t_context *ctx = malloc(sizeof(t_context));
    if (!ctx) {
        ft_fprintf(STDERR_FILENO, "error: failed to init context: %s\n", strerror(errno));
        return NULL;
    }

    ctx->options = init_options();
    ctx->host_addr = NULL;

    if (argc < 2) {
        print_helper(ctx->options);
        free_ctx(ctx);
        return NULL;
    }

    int err = parse_args(ctx, argc, argv);
    if (err == 2) {
        print_helper(ctx->options);
        free_ctx(ctx);
        exit(EXIT_SUCCESS);
    } else if (err != 0) {
        free_ctx(ctx);
        return NULL;
    }

    return ctx;
}

int main(int argc, char **argv) {
    t_context *ctx = init_ctx(argc, argv);
    if (!ctx) {
        return 1;
    }

    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1) {
        ft_fprintf(STDERR_FILENO, "error: failed to create socket: %s\n", strerror(errno));
        free_ctx(ctx);
        return 1;
    }

    struct sockaddr sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));

    sock_addr.sa_family = AF_INET;
    if (bind(sock_fd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) == -1) {
        ft_fprintf(STDERR_FILENO, "error: failed to bind socket: %s\n", strerror(errno));
        free_ctx(ctx);
        return 1;
    }

    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
        ft_fprintf(STDERR_FILENO, "error: failed to set socket options: %s\n", strerror(errno));
        free_ctx(ctx);
        return 1;
    }

    struct sockaddr_in *host_addr = (struct sockaddr_in *)ctx->host_addr;
    host_addr->sin_port = htons(33434);
    ft_printf("%s\n", inet_ntoa(((struct sockaddr_in *)ctx->host_addr)->sin_addr));
    ft_printf("%d\n", ntohs(host_addr->sin_port));

    int ttl = 1;
    if (setsockopt(sock_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) == -1) {
        ft_fprintf(STDERR_FILENO, "error: failed to set socket options: %s\n", strerror(errno));
        free_ctx(ctx);
        return 1;
    }

    char buf[5] = "test";
    if (sendto(sock_fd, buf, 5, 0, (struct sockaddr *)host_addr, ctx->host_addr_len) == -1) {
        ft_fprintf(STDERR_FILENO, "error: sendto failed: %s\n", strerror(errno));
        free_ctx(ctx);
        return 1;
    }

    fd_set read_fds;

    FD_ZERO(&read_fds);

    // int nfds = select(, , , NULL, );
    // if (nfds == -1) {
    //     ft_fprintf(STDERR_FILENO, "error: select failed: %s\n", strerror(errno));
    // }

    free_ctx(ctx);
    return 0;
}
