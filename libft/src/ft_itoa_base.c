/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa_base.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/28 10:18:13 by jlefonde          #+#    #+#             */
/*   Updated: 2024/03/29 10:29:04 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

static long	ft_get_sign(int nbr)
{
	long	nb;

	nb = (long)nbr;
	if (nb < 0)
		nb = -nb;
	return (nb);
}

static int	ft_is_base_valid(char *base)
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

char	*ft_itoa_base(int nbr, char *base)
{
	int		digits[10];
	int		i;
	int		j;
	long	nb;
	char	*result;

	if (!ft_is_base_valid(base))
		return (NULL);
	i = 0;
	nb = ft_get_sign(nbr);
	while (nb)
	{
		digits[i++] = (nb % ft_strlen(base));
		nb /= ft_strlen(base);
	}
	result = (char *)malloc((i + 2) * sizeof(char));
	if (result == NULL)
		return (NULL);
	j = 0;
	if (nbr < 0)
		result[j++] = '-';
	while (i > 0)
		result[j++] = base[digits[--i]];
	result[j] = '\0';
	return (result);
}
