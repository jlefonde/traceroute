#include "../include/libft.h"

uint32_t    ft_bswap32(uint32_t x)
{
    return (((x >> 24) & 0x000000FF) |
            ((x >> 16) & 0x0000FF00) |
            ((x << 16) & 0x00FF0000) |
            ((x << 24) & 0xFF000000));
}
