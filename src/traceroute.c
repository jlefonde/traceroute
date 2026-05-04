#define _POSIX_C_SOURCE 200112L

#include "../libft/include/libft.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>

typedef struct s_options
{
    const char short_opt;
    const char *long_opt;
    const char *description;
    bool has_argument;

    union
    {
        struct
        {
            const char *meta;
            char *value;
        } arg;

        struct
        {
            bool is_set;
        } flag;
    } data;
} t_options;

int parse_short_option(t_options *options, size_t option_len, char *option)
{
    for (int j = 1; j < ft_strlen(option); j++)
    {
        int option_idx = -1;
        char flag = option[j];

        for (int k = 0; k < option_len / sizeof(t_options); k++)
        {
            if (options[k].short_opt == flag)
            {
                option_idx = k;
                options[k].data.flag.is_set = true;
                break;
            }
        }

        if (option_idx == -1)
        {
            ft_fprintf(STDERR_FILENO, "ft_traceroute: bad option '-%c'\n", flag);
            return 1;
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    t_options options[] = {
        {
            .short_opt = 'h',
            .long_opt = "help",
            .description = "Show this help and exit",
        },
    };

    for (int i = 1; i < argc; i++)
    {
        int res;

        if (ft_strlen(argv[i]) > 1 && argv[i][0] == '-' && argv[i][1] != '-') {
            res = parse_short_option(options, sizeof(options), argv[i]);
        }

        if (res != 0) {
            exit(EXIT_FAILURE);
        }

        // if (!options[option_idx].has_argument) {
        // } else if ((i + 1) < argc) {
        //     options[option_idx].data.arg.value = argv[++i];
        // } else {
        //     ft_printf("arg required\n");
        // }
    }

    struct addrinfo *res, *rp;
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM,
        .ai_flags = AI_ADDRCONFIG,
    };

    int err_code = getaddrinfo(argv[1], NULL, &hints, &res);
    if (err_code != 0)
    {
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
