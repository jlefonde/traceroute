#include "../include/traceroute.h"

t_option *init_options() {
    static t_option options[] = {
        [OPT_HELP] = {
            .short_opt = 'h',
            .long_opt = "help",
            .description = "Print this help message and exit",
            .has_argument = false,
        },
        [OPT_FIRST_TTL] = {
            .short_opt = 'f',
            .long_opt = "first-ttl",
            .description = "Set the first TTL to start with",
            .has_argument = true,
            .meta = "first_ttl",
        },
        [OPT_MAX_TTL] = {
            .short_opt = 'm',
            .long_opt = "max-ttl",
            .description = "Set the maximum number of hops (TTL)",
            .has_argument = true,
            .meta = "max_ttl",
        },
        [OPT_QUERIES] = {
            .short_opt = 'q',
            .long_opt = "queries",
            .description = "Set the number of probe packets per hop",
            .has_argument = true,
            .meta = "nqueries",
        },
        [OPT_SIM_QUERIES] = {
            .short_opt = 'N',
            .long_opt = "sim-queries",
            .description = "Set the number of probe packets sent out simultaneously",
            .has_argument = true,
            .meta = "squeries",
        },
        [OPT_ICMP] = {
            .short_opt = 'I',
            .long_opt = "icmp",
            .description = "Use ICMP ECHO for tracerouting",
            .has_argument = false,
        },
        [OPT_NO_REVERSE] = {
            .short_opt = 'n',
            .long_opt = "",
            .description = "Disable reverse DNS lookup in hop display",
            .has_argument = false,
        },
    };

    return options;
}

int parse_short_option(t_option *options, char ***argv) {
    char *curr_str = **argv;
    int len = ft_strlen(curr_str);

    for (int i = 1; i < len; i++) {
        char short_opt = curr_str[i];

        t_option *option = get_option_short(options, short_opt);
        if (!option) {
            fprintf(stderr, "error: bad option '-%c'\n", short_opt);
            return -1;
        }

        if (!option->has_argument) {
            option->is_set = true;
            continue;
        }

        if (i == len - 1 && (*argv)[1]) {
            (*argv)++;
            option->value = **argv;
            option->is_set = true;
        }
        else {
            fprintf(stderr, "error: option '-%c' requires an argument\n", short_opt);
            return -1;
        }
    }

    return 0;
}

int parse_long_option(t_option *options, char ***argv) {
    char *long_opt = (**argv) + 2;

    t_option *option = get_option_long(options, long_opt);
    if (!option) {
        fprintf(stderr, "error: bad option '--%s'\n", long_opt);
        return -1;
    }

    if (!option->has_argument) {
        option->is_set = true;
    } else if ((*argv)[1]) {
        (*argv)++;
        option->value = **argv;
        option->is_set = true;
    } else {
        fprintf(stderr, "error: option '--%s' requires an argument\n", long_opt);
        return -1;
    }

    return 0;
}

t_option *get_option_short(t_option *options, char opt) {
    for (size_t i = 0; i < OPT_COUNT; i++) {
        if (options[i].short_opt == opt) {
            return &options[i];
        }
    }

    return NULL;
}

t_option *get_option_long(t_option *options, char *opt) {
    for (size_t i = 0; i < OPT_COUNT; i++) {
        if (options[i].long_opt[0] && ft_strcmp(options[i].long_opt, opt) == 0) {
            return &options[i];
        }
    }

    return NULL;
}

t_option *get_option_by_index(t_option *options, t_option_idx idx) {
    if (idx < 0 || idx >= OPT_COUNT) {
        return NULL;
    }

    return &options[idx];
}

char *get_option_value(t_option *option, t_option_idx idx) {
    if (!option || !option->has_argument) {
        return NULL;
    }

    return option->value;
}
