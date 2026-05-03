#include "../include/libft.h"

uint64_t    ft_rotate_left_64(uint64_t x, uint64_t n)
{
    return ((x << n) | (x >> (64 - n)));
}
