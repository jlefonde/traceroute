/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strchr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 13:20:51 by jlefonde          #+#    #+#             */
/*   Updated: 2024/03/19 11:41:48 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

/**
 * @brief Locates the first occurrence of a character in a string.
 *
 * @param s Pointer to the null-terminated string to search.
 * @param c The character to search for.
 * @return A pointer to the located character, otherwise NULL.
 */
char	*ft_strchr(const char *s, int c)
{
	unsigned char	ch;
	size_t			i;

	ch = (unsigned char)c;
	i = 0;
	while (s[i] && s[i] != ch)
		i++;
	if (s[i] == ch)
		return ((char *)(&s[i]));
	return (NULL);
}
