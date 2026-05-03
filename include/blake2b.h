# ifndef BLAKE2B_H
#  define BLAKE2B_H

# include "ssl.h"

typedef struct s_command t_command;

void process_blake2b(const t_command *cmd, int argc, char **argv);

# endif