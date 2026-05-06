#include "../include/traceroute.h"

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
        char short_opt = curr_str[i];

        t_option *option = get_option_short(ctx->options, short_opt);
        if (!option) {
            fprintf(stderr, "error: bad option '-%c'\n", short_opt);
            return -1;
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
            fprintf(stderr, "error: option '-%c' requires an argument\n", short_opt);
            return -1;
        }
    }

    return 0;
}

static int parse_long_option(t_context *ctx, char ***argv) {
    char *long_opt = (**argv) + 2;

    t_option *option = get_option_long(ctx->options, long_opt);
    if (!option) {
        fprintf(stderr, "error: bad option '--%s'\n", long_opt);
        return -1;
    }

    if (!option->has_argument) {
        option->data.flag.is_set = true;
    }
    else if ((*argv)[1]) {
        (*argv)++;
        option->data.arg.value = **argv;
    }
    else {
        fprintf(stderr, "error: option '--%s' requires an argument\n", long_opt);
        return -1;
    }

    return 0;
}

static int set_host_address(t_context *ctx, char *host_str) {
    if (!host_str) {
        fprintf(stderr, "error: missing \"host\" argument\n");
        return -1;
    }

    struct addrinfo *res;
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM,
        .ai_flags = AI_ADDRCONFIG,
    };

    int err_code = getaddrinfo(host_str, NULL, &hints, &res);
    if (err_code != 0) {
        fprintf(stderr, "error: '%s': %s\n", host_str, gai_strerror(err_code));
        return -1;
    }

    ctx->host_addr_len = res->ai_addrlen;
    ctx->host_addr = malloc(ctx->host_addr_len);
    if (!ctx->host_addr) {
        fprintf(stderr, "error: failed to allocate host address: %s\n", strerror(errno));
        freeaddrinfo(res);
        return -1;
    }

    if (res->ai_family == AF_INET) {
        ctx->packet_len = 60;
    } else if (res->ai_family == AF_INET6) {
        ctx->packet_len = 80;
    }

    ft_memcpy(ctx->host_addr, res->ai_addr, ctx->host_addr_len);

    
    freeaddrinfo(res);
    return 0;
}

void print_helper(t_option *options) {
    printf("usage: ./ft_traceroute <host> [options]\n");
    printf("options:\n");

    for (size_t i = 0; options[i].short_opt || (options[i].long_opt && options[i].long_opt[0]); i++) {
        int len = 0;
        
        if (options[i].short_opt && options[i].long_opt && options[i].long_opt[0]) {
            len = printf("  -%c, --%s", options[i].short_opt, options[i].long_opt);
        } else if (options[i].short_opt) {
            len = printf("  -%c", options[i].short_opt);
        } else if (options[i].long_opt && options[i].long_opt[0]) {
            len = printf("      --%s", options[i].long_opt);
        }

        if (options[i].has_argument && options[i].data.arg.meta) {
            len += printf(" <%s>", options[i].data.arg.meta);
        }

        if (len < 24) {
            printf("%*s", 24 - len, "");
        } else {
            printf(" ");
        }

        printf("%s\n", options[i].description ? options[i].description : "");
    }
}

int parse_args(t_context *ctx, int argc, char **argv) {
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
            fprintf(stderr, "error: extra argument '%s'\n", *arg);
            return -1;
        }

        if (err != 0) {
            return -1;
        }
    }
    
    if (set_host_address(ctx, host_str) != 0) {
        return -1;
    }

    return 0;
}
