/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_put.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/27 11:50:16 by jlefonde          #+#    #+#             */
/*   Updated: 2024/03/19 11:42:45 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/libft.h"

/**
 * @brief Outputs a character.
 *
 * @param c The character to output.
 * @return The length of a char.
 */
int	ft_putchar(char c)
{
	write(1, &c, 1);
	return (1);
}

/**
 * @brief Outputs a string.
 *
 * @param s The string to output.
 * @return The length of the string.
 */
int	ft_putstr(char *s)
{
	int	i;

	if (s == NULL)
		return (ft_putstr("(null)"));
	i = 0;
	while (s[i])
	{
		ft_putchar(s[i]);
		i++;
	}
	return (i);
}

/**
 * @brief Outputs an integer/unsigned integer.
 *
 * @param n The integer to output.
 * @param is_signed Flag for integer output.
 * @return The length of the integer.
 */
int	ft_putnbr(long long n, int is_signed)
{
	int	len;

	len = 0;
	if (n == -2147483648 && is_signed)
	{
		ft_putstr("-2147483648");
		return (11);
	}
	if (n < 0 && is_signed)
	{
		ft_putchar('-');
		n = -n;
		len++;
	}
	if (n >= 10)
		len += ft_putnbr(n / 10, is_signed);
	len += ft_putchar((n % 10) + '0');
	return (len);
}

/**
 * @brief Outputs an integer in uppercase or lowercase (hexadecimal).
 *
 * @param n The integer to output.
 * @param is_upper Flag for uppercase/lowercase output.
 * @return The length of the integer.
 */
int	ft_puthex(unsigned long int n, int is_upper)
{
	int	len;
	int	res;

	len = 0;
	if (n >= 16)
		len += ft_puthex(n / 16, is_upper);
	res = n % 16;
	if (res < 10)
		len += ft_putchar(res + '0');
	else if (is_upper)
		len += ft_putchar(res - 10 + 'A');
	else
		len += ft_putchar(res - 10 + 'a');
	return (len);
}

/**
 * @brief Outputs a pointer in hexadecimal.
 *
 * @param p The pointer to output.
 * @return The length of the pointer.
 */
int	ft_putptr(void *p)
{
	int	len;

	len = 0;
	if (p == NULL)
		return (ft_putstr("(nil)"));
	len += ft_putstr("0x");
	len += ft_puthex((unsigned long int)p, 0);
	return (len);
}
