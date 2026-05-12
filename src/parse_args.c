#include "traceroute.h"

void print_helper(t_option *options) {
    printf("usage: ./ft_traceroute <host> [options]\n");
    printf("options:\n");

    for (size_t i = 0; i < OPT_COUNT; i++) {
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

        if (len < HELPER_SPACE_LEN) {
            printf("%*s", HELPER_SPACE_LEN - len, "");
        } else {
            printf(" ");
        }

        printf("%s\n", options[i].description ? options[i].description : "");
    }
}

int parse_args(int argc, char **argv, t_option *options, char **host) {
    if (argc < 2) {
        print_helper(options);
        return -1;
    }

    for (char **arg = argv + 1; *arg; arg++) {
        int err = 0;

        if ((*arg)[0] == '-' && (*arg)[1] && (*arg)[1] != '-') {
            err = parse_short_option(options, &arg);
        }
        else if ((*arg)[0] == '-' && (*arg)[1] == '-' && (*arg)[2] && (*arg)[2] != '-') {
            err = parse_long_option(options, &arg);
        }
        else if (!(*host)) {
            *host = *arg;
        } else {
            fprintf(stderr, "error: extra argument '%s'\n", *arg);
            return -1;
        }

        if (err != 0) {
            return -1;
        }
    }

    if (!*host) {
        fprintf(stderr, "error: missing \"host\" argument\n");
        return -1;
    }

    t_option *help = get_option_by_index(options, OPT_HELP);
    if (help && help->data.flag.is_set) {
        print_helper(options);
        exit(EXIT_SUCCESS);
    }

    return 0;
}
