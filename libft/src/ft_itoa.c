/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/21 12:41:35 by jlefonde          #+#    #+#             */
/*   Updated: 2024/03/19 11:40:54 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

static int	count_digits(int n)
{
	int	count;

	count = 0;
	if (n <= 0)
		count++;
	while (n != 0)
	{
		n /= 10;
		count++;
	}
	return (count);
}

/**
 * @brief Converts an integer to a string.
 *
 * @param n The integer to convert.
 * @return The string representing the integer or NULL if the allocation fails.
 */
char	*ft_itoa(int n)
{
	int		digit_count;
	int		i;
	char	*str;
	long	nl;

	digit_count = count_digits(n);
	str = (char *)malloc((digit_count + 1) * sizeof(char));
	if (str == NULL)
		return (NULL);
	i = digit_count - 1;
	str[digit_count] = '\0';
	nl = (long)n;
	if (nl == 0)
		str[0] = '0';
	if (nl < 0)
	{
		str[0] = '-';
		nl = -nl;
	}
	while (nl)
	{
		str[i--] = (nl % 10) + '0';
		nl /= 10;
	}
	return (str);
}
