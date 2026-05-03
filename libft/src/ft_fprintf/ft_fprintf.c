/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_fprintf.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/27 11:49:49 by jlefonde          #+#    #+#             */
/*   Updated: 2024/05/02 09:49:10 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/libft.h"

int			g_strlen = 0;

/**
 * @brief Processes format specifier.
 *
 * @param format The format string.
 * @param args The variable argument list.
 * @param i Pointer to the current index in the format string.
 * @param fd The file descriptor.
 */
static void	ft_putf(char *format, va_list args, int *i, int fd)
{
	if (format[0] == 'c')
		ft_putchar_fd(va_arg(args, int), fd);
	else if (format[0] == 's')
		ft_putstr_fd(va_arg(args, char *), fd);
	else if (format[0] == 'p')
		ft_putptr_fd(va_arg(args, void *), fd);
	else if (format[0] == 'd' || format[0] == 'i')
		ft_putnbr_fd(va_arg(args, int), 1, fd);
	else if (format[0] == 'u')
		ft_putnbr_fd(va_arg(args, unsigned int), 0, fd);
	else if (format[0] == 'x')
		ft_puthex_fd((unsigned int)va_arg(args, int), 0, fd);
	else if (format[0] == 'X')
		ft_puthex_fd((unsigned int)va_arg(args, int), 1, fd);
	else if (format[0] == '%')
		ft_putchar_fd('%', fd);
	else if (format[0] == ' ' && ft_inset(format[1], "di"))
		ft_put_space_fd(args, i, fd);
	else if (format[0] == '+' && ft_inset(format[1], "di"))
		ft_put_plus_fd(args, i, fd);
	else if (format[0] == '#' && ft_inset(format[1], "xX"))
		ft_put_sharp_fd(args, i, format[1], fd);
}

/**
 * @brief Outputs the formatted string to the specified file descriptor.
 *
 * @param fd The file descriptor.
 * @param format The format string.
 * @param ... The variable arguments list.
 * @return The length of the string.
 */
int	ft_fprintf(int fd, const char *format, ...)
{
	va_list	args;
	int		i;

	va_start(args, format);
	i = 0;
	while (format[i])
	{
		if (format[i] == '%')
		{
			i++;
			ft_putf((char *)&format[i], args, &i, fd);
		}
		else
			ft_putchar_fd(format[i], fd);
		i++;
	}
	va_end(args);
	return (g_strlen);
}
