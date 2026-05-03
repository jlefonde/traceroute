/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/27 11:49:49 by jlefonde          #+#    #+#             */
/*   Updated: 2024/03/23 20:07:30 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/libft.h"

/**
 * @brief Processes format specifier.
 *
 * @param c The format specifier character.
 * @param args The variable argument list.
 * @param len The pointer to the length counter.
 */
static void	ft_putf(char *format, va_list args, int *len, int *i)
{
	if (format[0] == 'c')
		*len += ft_putchar(va_arg(args, int));
	else if (format[0] == 's')
		*len += ft_putstr(va_arg(args, char *));
	else if (format[0] == 'p')
		*len += ft_putptr(va_arg(args, void *));
	else if (format[0] == 'd' || format[0] == 'i')
		*len += ft_putnbr(va_arg(args, int), 1);
	else if (format[0] == 'u')
		*len += ft_putnbr(va_arg(args, unsigned int), 0);
	else if (format[0] == 'x' || format[0] == 'X')
		*len += ft_puthex((unsigned int)va_arg(args, int), format[0] == 'X');
	else if (format[0] == '%')
		*len += ft_putchar('%');
	else if (format[0] == ' ' && ft_inset(format[1], "di"))
		ft_put_space(args, len, i);
	else if (format[0] == '+' && ft_inset(format[1], "di"))
		ft_put_plus(args, len, i);
	else if (format[0] == '#' && ft_inset(format[1], "xX"))
		ft_put_sharp(args, len, i, format[1]);
	else if (format[0] == '0')
		ft_put_zero(args, len, i, format);
}

/**
 * @brief Outputs the formatted string.
 *
 * @param format The format string.
 * @param ... The variable arguments list.
 * @return The length of the string.
 */
int	ft_printf(const char *format, ...)
{
	va_list	args;
	int		i;
	int		len;

	va_start(args, format);
	len = 0;
	i = 0;
	while (format[i])
	{
		if (format[i] == '%')
		{
			i++;
			ft_putf((char *)&format[i], args, &len, &i);
		}
		else
			len += ft_putchar(format[i]);
		i++;
	}
	va_end(args);
	return (len);
}
