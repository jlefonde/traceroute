/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memchr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 12:06:39 by jlefonde          #+#    #+#             */
/*   Updated: 2024/03/19 11:41:18 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

/**
 * @brief Locates the first occurrence of a byte in a byte string.
 *
 * @param s Pointer to the memory area to be searched.
 * @param c The byte to be located.
 * @param n Number of bytes to be searched.
 * @return A pointer to the located byte, or NULL if the byte is not found.
 */
void	*ft_memchr(const void *s, int c, size_t n)
{
	unsigned char	*p;
	unsigned char	ch;
	size_t			i;

	ch = (unsigned char)c;
	p = (unsigned char *)s;
	i = 0;
	while (i < n)
	{
		if (p[i] == ch)
			return ((void *)(&p[i]));
		i++;
	}
	return (NULL);
}
