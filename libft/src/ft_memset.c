/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memset.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/19 13:05:02 by jlefonde          #+#    #+#             */
/*   Updated: 2024/03/19 11:41:29 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

/**
 * @brief Fills a block of memory with a specified value.
 *
 * @param s Pointer to the memory block to fill.
 * @param c The value to be set (interpreted as an unsigned char).
 * @param n Number of bytes to be set to the value.
 * @return A pointer to the memory area ptr.
 */
void	*ft_memset(void *s, int c, size_t n)
{
	unsigned char	*p;
	size_t			i;

	p = s;
	i = 0;
	while (i < n)
	{
		p[i] = (unsigned char)c;
		i++;
	}
	return (s);
}
