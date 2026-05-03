/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strjoin.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/21 09:44:49 by jlefonde          #+#    #+#             */
/*   Updated: 2024/03/19 11:41:56 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

/**
 * @brief Concatenates two strings.
 *
 * @param s1 The prefix string.
 * @param s2 The suffix string.
 * @return The new string or NULL if the allocation fails.
 */
char	*ft_strjoin(char const *s1, char const *s2)
{
	char	*s3;
	int		s1_len;
	int		s2_len;
	int		s3_len;

	if (s1 == NULL || s2 == NULL)
		return (NULL);
	s1_len = ft_strlen(s1);
	s2_len = ft_strlen(s2);
	s3_len = s1_len + s2_len;
	s3 = (char *)malloc((s3_len + 1) * sizeof(char));
	if (s3 == NULL)
		return (NULL);
	ft_strlcpy(s3, s1, s1_len + 1);
	ft_strlcat(s3 + s1_len, s2, s2_len + 1);
	return (s3);
}
