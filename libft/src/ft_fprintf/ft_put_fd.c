/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_put_fd.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/27 11:50:16 by jlefonde          #+#    #+#             */
/*   Updated: 2024/05/13 19:27:26 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/libft.h"

/**
 * @brief Outputs a character to the specified file descriptor.
 *
 * @param c The character to output.
 * @param fd The file descriptor.
 */
void	ft_putchar_fd(char c, int fd)
{
	write(fd, &c, 1);
	g_strlen++;
}

/**
 * @brief Outputs a string to the specified file descriptor.
 *
 * @param s The string to output.
 * @param fd The file descriptor.
 */
void	ft_putstr_fd(char *s, int fd)
{
	if (s == NULL)
	{
		ft_putstr_fd("(null)", fd);
		return ;
	}
	while (*s)
	{
		ft_putchar_fd(*s, fd);
		s++;
	}
}

/**
 * @brief Outputs an int/unsigned int to the specified file descriptor.
 *
 * @param n The integer to output.
 * @param is_signed Flag for integer output.
 * @param fd The file descriptor.
 */
void	ft_putnbr_fd(long long n, int is_signed, int fd)
{
	if (n == -2147483648 && is_signed)
	{
		ft_putstr_fd("-2147483648", fd);
		return ;
	}
	if (n < 0 && is_signed)
	{
		ft_putchar_fd('-', fd);
		n = -n;
	}
	if (n >= 10)
		ft_putnbr_fd(n / 10, is_signed, fd);
	ft_putchar_fd((n % 10) + '0', fd);
}

/**
 * @brief Outputs a hexadecimal to the specified file descriptor.
 *
 * @param n The integer to output.
 * @param is_upper Flag for uppercase/lowercase output.
 * @param fd The file descriptor.
 */
void	ft_puthex_fd(unsigned long int n, int is_upper, int fd)
{
	int	res;

	if (n >= 16)
		ft_puthex_fd(n / 16, is_upper, fd);
	res = n % 16;
	if (res < 10)
		ft_putchar_fd(res + '0', fd);
	else if (is_upper)
		ft_putchar_fd(res - 10 + 'A', fd);
	else
		ft_putchar_fd(res - 10 + 'a', fd);
}

/**
 * @brief Outputs a pointer in hexadecimal to the specified file descriptor.
 *
 * @param p The pointer to output.
 * @param fd The file descriptor.
 */
void	ft_putptr_fd(void *p, int fd)
{
	if (p == NULL)
	{
		ft_putstr_fd("(nil)", fd);
		return ;
	}
	ft_putstr_fd("0x", fd);
	ft_puthex_fd((unsigned long int)p, 0, fd);
}
