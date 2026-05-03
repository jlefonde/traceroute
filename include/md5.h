# ifndef MD5_H
#  define MD5_H

# include "ssl.h"

typedef struct s_md5_round
{
    uint32_t A;
    uint32_t B;
    uint32_t C;
    uint32_t D;
    uint32_t w[16];
}   t_md5_round;

typedef struct s_command t_command;

void process_md5(const t_command *cmd, int argc, char **argv);

# endif