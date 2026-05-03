#define _POSIX_C_SOURCE 200112L

#include "../libft/include/libft.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    if (argc < 2) {
        return 1;
    }

    struct addrinfo *res, *rp;
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM,
        .ai_flags = AI_PASSIVE,
        .ai_protocol = 0,
        .ai_canonname = NULL,
    };

    int s = getaddrinfo(argv[1], NULL, &hints, &res);
    if (s != 0)
    {
        ft_fprintf(STDERR_FILENO, "ft_traceroute: getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    ft_printf("%s\n", inet_ntoa(((struct sockaddr_in *)res->ai_addr)->sin_addr));

    freeaddrinfo(res);
    return 0;
}
