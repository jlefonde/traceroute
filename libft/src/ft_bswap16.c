#include "../include/libft.h"

uint16_t    ft_bswap16(uint16_t x)
{
    return ((x >> 8) | (x << 8));
}
