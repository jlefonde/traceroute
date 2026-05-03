/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memmove.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 10:49:37 by jlefonde          #+#    #+#             */
/*   Updated: 2024/03/19 11:41:26 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

/**
 * @brief Copies data from one memory area to another, handling overlapping.
 *
 * @param dest Pointer to the destination arr where the content is to be copied.
 * @param src Pointer to the source of data to be copied.
 * @param n Number of bytes to copy.
 * @return A pointer to dest.
 */
void	*ft_memmove(void *dest, const void *src, size_t n)
{
	unsigned char		*p_dest;
	const unsigned char	*p_src;
	size_t				i;

	p_dest = (unsigned char *)dest;
	p_src = (const unsigned char *)src;
	if (dest == src)
		return (dest);
	if (p_dest < p_src)
	{
		i = 0;
		while (i < n)
		{
			p_dest[i] = p_src[i];
			i++;
		}
	}
	else
	{
		i = n;
		while (i-- > 0)
			p_dest[i] = p_src[i];
	}
	return (dest);
}
