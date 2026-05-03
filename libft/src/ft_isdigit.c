/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_isdigit.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/19 12:17:20 by jlefonde          #+#    #+#             */
/*   Updated: 2024/03/19 11:40:49 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

/**
 * @brief Checks whether a character is a decimal digit (0-9).
 *
 * @param c The character to check.
 * @return Non-zero value if the character is a digit, 0 otherwise.
 */
int	ft_isdigit(int c)
{
	return (c >= 48 && c <= 57);
}
