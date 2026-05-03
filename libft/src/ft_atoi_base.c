/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoi_base.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 20:05:14 by jlefonde          #+#    #+#             */
/*   Updated: 2024/03/26 14:32:34 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

static int	ft_char_index(char c, char *base)
{
	int	i;

	i = 0;
	while (base[i])
	{
		if (base[i] == c)
			return (i);
		i++;
	}
	return (-1);
}

static int	ft_isvalid_base(char *base)
{
	int	i;
	int	j;

	i = 0;
	if (ft_strlen(base) <= 1)
		return (0);
	while (base[i])
	{
		if (base[i] == '+' || base[i] == '-')
			return (0);
		j = i + 1;
		while (base[j])
		{
			if (base[i] == base[j])
				return (0);
			j++;
		}
		i++;
	}
	return (1);
}

static void	ft_skip_sign(char c, int *i, int *sign)
{
	if (c == '+' || c == '-')
	{
		if (c == '-')
			*sign = -1;
		(*i)++;
	}
}

int	ft_atoi_base(char *str, char *base)
{
	int	i;
	int	sign;
	int	number;
	int	base_len;

	i = 0;
	sign = 1;
	number = 0;
	base_len = ft_strlen(base);
	if (!ft_isvalid_base(base))
		return (0);
	if (ft_strlen(str) > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
		i = 2;
	while (str[i] == 32 || (str[i] >= 9 && str[i] <= 13))
		i++;
	ft_skip_sign(str[i], &i, &sign);
	while (str[i] && ft_char_index(ft_toupper(str[i]), base) != -1)
	{
		number = number * base_len + ft_char_index(ft_toupper(str[i]), base);
		i++;
	}
	return (number * sign);
}
