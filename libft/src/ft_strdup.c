/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strdup.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 13:09:03 by jlefonde          #+#    #+#             */
/*   Updated: 2024/03/19 11:41:52 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

/**
 * @brief Duplicates a string.
 *
 * @param s Pointer to the null-terminated string to duplicate.
 * @return Pointer to the newly allocated string, identical to str.
 *         Returns NULL if allocation fails.
 */
char	*ft_strdup(const char *s)
{
	size_t	s_len;
	char	*dest;

	s_len = ft_strlen(s) + 1;
	dest = (char *)malloc(s_len);
	if (dest == NULL)
		return (NULL);
	ft_strlcpy(dest, s, s_len);
	return (dest);
}
