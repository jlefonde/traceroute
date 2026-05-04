#define _POSIX_C_SOURCE 200112L

#include "../libft/include/libft.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdbool.h>
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

static t_option options[] = {
    {
        .short_opt = '4',
        .long_opt = "4",
        .description = "Show this help and exit",
        .has_argument = false,
    },
    {
        .short_opt = 'i',
        .long_opt = "i",
        .description = "Show this help and exit",
        .has_argument = true,
        .data.arg.meta = "device",
    },
    {
        .short_opt = 'g',
        .long_opt = "g",
        .description = "Show this help and exit",
        .has_argument = true,
        .data.arg.meta = "gate,...",
    },
};

static t_option *get_option_short(char opt) {
    for (size_t i = 0; i < sizeof(options) / sizeof(t_option); i++) {
        if (options[i].short_opt == opt) {
            return &options[i];
        }
    }

    return NULL;
}

static t_option *get_option_long(char *opt) {
    for (size_t i = 0; i < sizeof(options) / sizeof(t_option); i++) {
        if (ft_strcmp(options[i].long_opt, opt) == 0) {
            return &options[i];
        }
    }

    return NULL;
}

int parse_short_option(char ***argv) {
    char *curr_str = **argv;
    int len = ft_strlen(curr_str);

    for (int i = 1; i < len; i++) {
        int option_idx = -1;
        char short_opt = curr_str[i];

        t_option *option = get_option_short(short_opt);
        if (!option) {
            ft_fprintf(STDERR_FILENO, "ft_traceroute: bad option '-%c'\n", short_opt);
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
            ft_fprintf(STDERR_FILENO, "ft_traceroute: option '-%c' requires an argument\n", short_opt);
            return 1;
        }
    }

    return 0;
}

int parse_long_option(char ***argv) {
    int option_idx = -1;
    char *long_opt = (**argv) + 2;

    t_option *option = get_option_long(long_opt);
    if (!option) {
        ft_fprintf(STDERR_FILENO, "ft_traceroute: bad option '-%s'\n", long_opt);
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
        ft_fprintf(STDERR_FILENO, "ft_traceroute: option '--%s' requires an argument\n", long_opt);
        return 1;
    }

    return 0;
}

int parse_args(int argc, char **argv) {
    for (char **curr = argv + 1; *curr; curr++) {
        int err;

        if (ft_strlen(*curr) > 1 && (*curr)[0] == '-' && (*curr)[1] != '-') {
            err = parse_short_option(&curr);
        }
        else if (ft_strlen(*curr) > 2 && (*curr)[0] == '-' && (*curr)[1] == '-' && (*curr)[2] != '-') {
            err = parse_long_option(&curr);
        }
        else {
            ft_printf("Host: %s\n", *curr);
        }

        if (err != 0) {
            return 1;
        }
    }

    return 0;
}

int main(int argc, char **argv) {
    int err = parse_args(argc, argv);
    if (err != 0) {
        exit(EXIT_FAILURE);
    }

    struct addrinfo *res;
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM,
        .ai_flags = AI_ADDRCONFIG,
    };

    int err_code = getaddrinfo(argv[1], NULL, &hints, &res);
    if (err_code != 0) {
        ft_fprintf(STDERR_FILENO, "ft_traceroute: getaddrinfo: %s\n", gai_strerror(err_code));
        exit(EXIT_FAILURE);
    }

    char host[1025], service[32];
    int s2 = getnameinfo(res->ai_addr, res->ai_addrlen, host, sizeof(host), service, sizeof(service), 0);
    ft_printf("host=%s, serv=%s\n", host, service);

    ft_printf("%s\n", inet_ntoa(((struct sockaddr_in *)res->ai_addr)->sin_addr));

    freeaddrinfo(res);
    return 0;
}
