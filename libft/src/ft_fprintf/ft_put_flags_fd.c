/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_put_flags_fd.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/29 11:27:57 by jlefonde          #+#    #+#             */
/*   Updated: 2024/05/13 19:27:23 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/libft.h"

/**
 * @brief Adds a space before positive numbers.
 *
 * @param args The variable argument list.
 * @param i The pointer to the index counter.
 * @param fd The file descriptor.
 */
void	ft_put_space_fd(va_list args, int *i, int fd)
{
	int	n;

	n = va_arg(args, int);
	if (n >= 0)
		ft_putstr_fd(" ", fd);
	ft_putnbr_fd(n, 1, fd);
	*i = *i + 1;
}

/**
 * @brief Adds a plus before positive numbers.
 *
 * @param args The variable argument list.
 * @param i The pointer to the index counter.
 * @param fd The file descriptor.
 */
void	ft_put_plus_fd(va_list args, int *i, int fd)
{
	int	n;

	n = va_arg(args, int);
	if (n >= 0)
		ft_putstr_fd("+", fd);
	ft_putnbr_fd(n, 1, fd);
	*i = *i + 1;
}

/**
 * @brief Adds the hexadecimal prefix.
 *
 * @param args The variable argument list.
 * @param i The pointer to the index counter.
 * @param c The character specifying the prefix ('x' or 'X').
 * @param fd The file descriptor.
 */
void	ft_put_sharp_fd(va_list args, int *i, char c, int fd)
{
	unsigned int	n;

	n = (unsigned int)va_arg(args, int);
	if (n > 0)
	{
		if (c == 'x')
			ft_putstr_fd("0x", fd);
		else if (c == 'X')
			ft_putstr_fd("0X", fd);
	}
	ft_puthex_fd(n, (c == 'X'), fd);
	*i = *i + 1;
}
