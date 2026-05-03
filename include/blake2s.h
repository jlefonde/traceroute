# ifndef BLAKE2S_H
#  define BLAKE2S_H

# include "ssl.h"

typedef struct s_command t_command;

void process_blake2s(const t_command *cmd, int argc, char **argv);

# endif