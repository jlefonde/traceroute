#include "traceroute.h"

#define MAX_TTL_VALUE 255
#define MIN_TTL_VALUE 1

static bool parse_max_ttl(const char *str, void *value)
{
    if (!str) {
        return false;
    }

    int max_ttl = ft_atoi(str);
    if (!max_ttl) {
        printf("error: invalid max_ttl\n");
        return false;
    }

    if (max_ttl < MIN_TTL_VALUE || max_ttl > MAX_TTL_VALUE) {
        printf("error: max_ttl out of range [%d,%d]\n", MIN_TTL_VALUE, MAX_TTL_VALUE);
        return false;
    }

    *((int *)value) = max_ttl;
    return true;
}

t_option *init_options() {
    static size_t max_ttl = 30;

    static t_option options[] = {
        [OPT_HELP] = {
            .short_opt = 'h',
            .long_opt = "help",
            .description = "Print this help message and exit",
            .has_argument = false,
        },
        [OPT_NO_REVERSE] = {
            .short_opt = 'n',
            .long_opt = "",
            .description = "Do not perform reverse DNS lookup in hop display",
            .has_argument = false,
        },
        [OPT_MAX_TTL] = {
            .short_opt = 'm',
            .long_opt = "max-ttl",
            .description = "Specifies the maximum number of hops (time-to-live) to be performed: [1,255]",
            .has_argument = true,
            .data.arg.meta = "max_ttl",
            .data.arg.value = &max_ttl,
            .data.arg.parser = parse_max_ttl,
        },
    };

    return options;
}

static bool set_option_value(t_option *option, char *argv) {
    option->data.arg.raw = argv;
    if (option->data.arg.parser) {
        return option->data.arg.parser(option->data.arg.raw, option->data.arg.value);
    }
    
    option->data.arg.value = option->data.arg.raw;
    return true;
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
            option->data.flag.is_set = true;
            continue;
        }

        if (i == len - 1 && (*argv)[1]) {
            (*argv)++;
            if (!set_option_value(option, **argv)) {
                return -1;
            }
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
        option->data.flag.is_set = true;
    }
    else if ((*argv)[1]) {
        (*argv)++;
        if (!set_option_value(option, **argv)) {
            return -1;
        }
    }
    else {
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

void *get_option_value(t_option *options, t_option_idx idx) {
    t_option *opt = get_option_by_index(options, idx);
    if (!opt || !opt->has_argument) {
        return NULL;
    }

    return opt->data.arg.value;
}
