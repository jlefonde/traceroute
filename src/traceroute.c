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
    struct in_addr *host;
    t_option *options;
} t_context;

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

static struct in_addr *get_host_address(char *host_str) {
    struct addrinfo *res;
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM,
        .ai_flags = AI_ADDRCONFIG,
    };

    int err_code = getaddrinfo(host_str, NULL, &hints, &res);
    if (err_code != 0) {
        ft_fprintf(STDERR_FILENO, "error: '%s': %s\n", host_str, gai_strerror(err_code));
        return NULL;
    }
    
    struct in_addr *host = malloc(sizeof(struct in_addr));
    if (!host) {
        ft_fprintf(STDERR_FILENO, "error: failed to retrieve host address: %s\n", strerror(errno));
        return NULL;
    }

    *host = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
    freeaddrinfo(res);

    return host;
}

static int set_host_address(t_context *ctx, char *host_str) {
    if (ctx->host) {
        ft_fprintf(STDERR_FILENO, "error: extra argument '%s'\n", host_str);
        return 1;
    }

    ctx->host = get_host_address(host_str);
    if (!ctx->host) {
        return 1;
    }

    return 0;
}


static int parse_args(t_context *ctx, int argc, char **argv) {
    char **curr = argv + 1;
    for (curr; *curr; curr++) {
        int err = 0;

        if (ft_strlen(*curr) > 1 && (*curr)[0] == '-' && (*curr)[1] != '-') {
            err = parse_short_option(ctx, &curr);
        }
        else if (ft_strlen(*curr) > 2 && (*curr)[0] == '-' && (*curr)[1] == '-' && (*curr)[2] != '-') {
            err = parse_long_option(ctx, &curr);
        }
        else if (!ctx->host) {
            err = set_host_address(ctx, *curr);
        } else {
            ft_fprintf(STDERR_FILENO, "error: extra argument '%s'\n", *curr);
            return 1;
        }

        if (err != 0) {
            return 1;
        }
    }

    if (!ctx->host) {
        // TODO: show usage
        return 1;
    }

    return 0;
}

static t_option *init_options() {
    static t_option options[] = {
        {
            .short_opt = 'h',
            .long_opt = "help",
            .description = "Show this help and exit",
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
    ctx->host = NULL;

    int err = parse_args(ctx, argc, argv);
    if (err != 0) {
        free(ctx->host);
        free(ctx);
        return NULL;
    }

    return ctx;
}

int main(int argc, char **argv) {
    t_context *ctx = init_ctx(argc, argv);
    if (!ctx) {
        return 1;
    }

    ft_printf("%s\n", inet_ntoa(*ctx->host));
    free(ctx->host);
    free(ctx);
    return 0;
}
