/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/21 10:50:22 by jlefonde          #+#    #+#             */
/*   Updated: 2024/03/23 18:23:37 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

static void	find_next(const char *s, char c, size_t *start, size_t *end)
{
	while (s[*start] && s[*start] == c)
		(*start)++;
	*end = *start;
	while (s[*end] && s[*end] != c)
		(*end)++;
}

/**
 * @brief Splits a string into an arr of substrings based on a delimiter char.
 *
 * @param s The string to be split.
 * @param c The delimiter character.
 * @return The array of new strings resulting from the split or NULL if failed.
 */
char	**ft_split(char const *s, char c)
{
	char	**result;
	int		word_count;
	int		i;
	size_t	start;
	size_t	end;

	if (s == NULL)
		return (NULL);
	word_count = ft_count_words(s, c);
	result = (char **)malloc((word_count + 1) * sizeof(char *));
	if (result == NULL)
		return (NULL);
	i = 0;
	start = 0;
	while (s[start] && i < word_count)
	{
		find_next(s, c, &start, &end);
		result[i] = ft_substr(s, start, end - start);
		if (result[i] == NULL)
			return (NULL);
		i++;
		start = end;
	}
	result[i] = NULL;
	return (result);
}
