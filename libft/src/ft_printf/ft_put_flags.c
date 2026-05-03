/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_put_flags.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/29 11:27:57 by jlefonde          #+#    #+#             */
/*   Updated: 2024/03/19 11:42:43 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/libft.h"

/**
 * @brief Adds a space before positive numbers.
 *
 * @param args The variable argument list.
 * @param len The pointer to the length counter.
 * @param i The pointer to the index counter.
 */
void	ft_put_space(va_list args, int *len, int *i)
{
    int	n;

    n = va_arg(args, int);
    if (n >= 0)
        *len += ft_putstr(" ");
    *len += ft_putnbr(n, 1);
    *i = *i + 1;
}

/**
 * @brief Adds a plus before positive numbers.
 *
 * @param args The variable argument list.
 * @param len The pointer to the length counter.
 * @param i The pointer to the index counter.
 */
void	ft_put_plus(va_list args, int *len, int *i)
{
    int	n;

    n = va_arg(args, int);
    if (n >= 0)
        *len += ft_putstr("+");
    *len += ft_putnbr(n, 1);
    *i = *i + 1;
}

/**
 * @brief Adds the hexadecimal prefix.
 *
 * @param args The variable argument list.
 * @param len The pointer to the length counter.
 * @param i The pointer to the index counter.
 * @param c The character specifying the prefix ('x' or 'X').
 */
void	ft_put_sharp(va_list args, int *len, int *i, char c)
{
    unsigned int	n;

    n = (unsigned int)va_arg(args, int);
    if (n > 0)
    {
        if (c == 'x')
            *len += ft_putstr("0x");
        else if (c == 'X')
            *len += ft_putstr("0X");
    }
    *len += ft_puthex(n, (c == 'X'));
    *i = *i + 1;
}

static void ft_put_padding(char pad, int expected_width, int current_width)
{
    int n = (expected_width < current_width) ? 0 : expected_width - current_width;

    while (n > 0)
    {
        ft_putchar(pad);
        n--;
    }
}

static int ft_num_digits_base(long n, long base)
{
    int i = 1;
    while (n >= base)
    {
        n /= base;
        i++;
    }
    return (i);
}

void	ft_put_zero(va_list args, int *len, int *i, char *format)
{
    int j = 1;
    while (ft_isdigit(format[j]))
        j++;

    long width = 0;
    if (j > 1)
    {
        char *width_str = malloc(j);
        if (!width_str)
            return ;
        ft_strlcpy(width_str, &format[1], j);
        width = ft_atol(width_str);
        width = width > 2147483647 ? 0 : width;
        free(width_str);
    }

    int length = 0;
    if (format[j] == 'd' || format[j] == 'i')
    {
        int n = va_arg(args, int);
        int abs_n = (n < 0) ? -n : n;
        length = ft_num_digits_base(abs_n, 10) + (n < 0 ? 1 : 0);
        if (n < 0)
            *len += ft_putchar('-');
        ft_put_padding('0', width, length);
        *len += ft_putnbr(abs_n, 1);
    }
    else if (format[j] == 'u')
    {
        unsigned int n = va_arg(args, unsigned int);
        length = ft_num_digits_base(n, 10);
        ft_put_padding('0', width, length);
        *len += ft_putnbr(n, 0);
    }
    else if (format[j] == 'x' || format[j] == 'X')
    {
        unsigned int n = va_arg(args, unsigned int);
        length = ft_num_digits_base(n, 16);
        ft_put_padding('0', width, length);
        *len += ft_puthex(n, format[j] == 'X');
    }
    *i = *i + j;
}
