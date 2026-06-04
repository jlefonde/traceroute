#include "../include/traceroute.h"

int main(int argc, char **argv) {
    if (getuid() != ROOT_UID) {
        fprintf(stderr, "error: root privileges are required\n");
        return 1;
    }

    t_option *options = init_options();

    char *host = NULL;
    int err = parse_args(argc, argv, options, &host);
    if (err != 0) {
        return 1;
    }

    t_context *ctx = init_ctx(options, host);
    if (!ctx) {
        return 1;
    }

    char *host_ip = inet_ntoa(((struct sockaddr_in *)ctx->host_addr)->sin_addr);
    printf("traceroute to %s (%s), %d hops max, %ld bytes packets\n", ctx->host_str, host_ip, ctx->max_ttl, ctx->packet_len);

    fd_set read_fds; 
    struct timeval timeout;

    while (1) {
        while (send_probe(ctx) > 0);

        FD_ZERO(&read_fds);
        FD_SET(ctx->icmp_sock->fd, &read_fds);

        set_timeout(ctx, &timeout);

        if (select(ctx->icmp_sock->fd + 1, &read_fds, NULL, NULL, &timeout) > 0) {
            recv_icmp_reply(ctx);
        }

        update_active_queries(ctx);
        print_available_hops(ctx);

        if ((ctx->unreached_port_ttl > 0 && ctx->next_hop >= ctx->unreached_port_ttl) || 
                ctx->unreachable_hop ||
                ctx->next_hop >= ctx->max_ttl) {
            break;
        }
    }

    free_ctx(ctx);
    return 0;
}
