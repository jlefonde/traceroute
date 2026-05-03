/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_isvalid_extension.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/23 16:27:17 by jlefonde          #+#    #+#             */
/*   Updated: 2024/03/26 14:33:40 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/libft.h"

/**
 * @brief Checks if a file has a valid extension.
 *
 * @param file The path to the file to check.
 * @param ext The valid extension.
 * @return A positive value if the extension is valid, 0 or negative otherwise.
 */
int	ft_isvalid_extension(char *file, char *ext)
{
	char	*file_ext;

	file_ext = ft_strrchr(file, '.');
	if (file_ext == NULL)
		return (-1);
	return (ft_strncmp(file_ext, ext, ft_strlen(ext)));
}
