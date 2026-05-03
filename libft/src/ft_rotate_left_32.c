#include "../include/libft.h"

uint32_t    ft_rotate_left_32(uint32_t x, uint32_t n)
{
    return ((x << n) | (x >> (32 - n)));
}
