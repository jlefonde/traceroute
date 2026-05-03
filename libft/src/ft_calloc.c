/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_calloc.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/20 12:53:01 by jlefonde          #+#    #+#             */
/*   Updated: 2024/03/19 11:40:30 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

/**
 * @brief Allocates memory for an arr of elements and initializes them to zero.
 *
 * @param nmemb Number of elements to allocate.
 * @param size Size of each element in bytes.
 * @return Pointer to the allocated memory or NULL if allocation fails.
 */
void	*ft_calloc(size_t nmemb, size_t size)
{
	size_t	total_size;
	void	*res;

	total_size = nmemb * size;
	if (nmemb != 0 && (total_size / nmemb) != size)
		return (NULL);
	res = (void *)malloc(total_size);
	if (res == NULL)
		return (NULL);
	ft_bzero(res, total_size);
	return (res);
}
