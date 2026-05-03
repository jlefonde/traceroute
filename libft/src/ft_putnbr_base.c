/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putnbr_base.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/28 10:36:05 by jlefonde          #+#    #+#             */
/*   Updated: 2024/03/28 10:36:11 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

static long	ft_print_sign(int nbr)
{
	long	nb;

	nb = (long)nbr;
	if (nb < 0)
	{
		ft_putchar('-');
		nb = -nb;
	}
	if (nb == 0)
		ft_putchar('0');
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

void	ft_putnbr_base(int nbr, char *base)
{
	int		digits[10];
	int		i;
	long	nb;

	if (ft_is_base_valid(base))
	{
		i = 0;
		nb = ft_print_sign(nbr);
		while (nb)
		{
			digits[i++] = (nb % ft_strlen(base));
			nb /= ft_strlen(base);
		}
		i--;
		while (i >= 0)
		{
			ft_putchar(base[digits[i]]);
			i--;
		}
	}
}
