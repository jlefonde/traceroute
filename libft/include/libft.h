/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   libft.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jlefonde <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/19 11:43:46 by jlefonde          #+#    #+#             */
/*   Updated: 2024/04/11 12:41:31 by jlefonde         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LIBFT_H
# define LIBFT_H

# include <strings.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# include <stdio.h>
# include <stdarg.h>
# include <stdint.h>

typedef struct s_list
{
    void			*content;
    struct s_list	*next;
}	t_list;

int         ft_isalpha(int c);
int         ft_isdigit(int c);
int         ft_isalnum(int c);
int         ft_isascii(int c);
int         ft_isprint(int c);
int         ft_toupper(int c);
int         ft_tolower(int c);
int         ft_strcmp(const char *s1, const char *s2);
int         ft_strncmp(const char *s1, const char *s2, size_t n);
int         ft_strcasecmp(const char *s1, const char *s2);
int         ft_strncasecmp(const char *s1, const char *s2, size_t n);
int         ft_memcmp(const void *s1, const void *s2, size_t n);
int         ft_atoi(const char *nptr);
int         ft_atoi_base(char *str, char *base);
int         ft_lstsize(t_list *lst);
int         ft_lerp(int a, int b, float t);
int         ft_isvalid_extension(char *file, char *ext);
int         ft_count_words(const char *s, char c);
int         ft_inset(char c, char *set);
size_t      ft_strlen(const char *s);
size_t      ft_strlcpy(char *dst, const char *src, size_t size);
size_t      ft_strlcat(char *dst, const char *src, size_t size);
void        ft_bzero(void *s, size_t n);
void        ft_striteri(char *s, void (*f)(unsigned int, char*));
void        ft_lstadd_front(t_list **lst, t_list *new);
void        ft_lstadd_back(t_list **lst, t_list *new);
void        ft_lstdelone(t_list *lst, void (*del)(void*));
void        ft_lstclear(t_list **lst, void (*del)(void*));
void        ft_lstiter(t_list *lst, void (*f)(void *));
void        ft_putnbr_base(int nbr, char *base);
void        *ft_memset(void *s, int c, size_t n);
void        *ft_memcpy(void *dest, const void *src, size_t n);
void        *ft_memmove(void *dest, const void *src, size_t n);
void        *ft_memchr(const void *s, int c, size_t n);
void        *ft_calloc(size_t nmemb, size_t size);
char        *ft_strchr(const char *s, int c);
char        *ft_strrchr(const char *s, int c);
char        *ft_strnstr(const char *big, const char *little, size_t len);
char        *ft_strdup(const char *s);
char        *ft_substr(char const *s, unsigned int start, size_t len);
char        *ft_strjoin(char const *s1, char const *s2);
char        *ft_strtrim(char const *s1, char const *set);
char        *ft_itoa(int n);
char        *ft_strmap(char const *s, int (*f)(int));
char        *ft_strmapi(char const *s, char (*f)(unsigned int, char));
char        *ft_itoa_base(int nbr, char *base);
char        **ft_split(char const *s, char c);
t_list      *ft_lstnew(void *content);
t_list      *ft_lstlast(t_list *lst);
t_list      *ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *));
long        ft_atol(const char *nptr);
uint16_t    ft_bswap16(uint16_t x);
uint32_t    ft_bswap32(uint32_t x);
uint64_t    ft_bswap64(uint64_t x);
uint32_t    ft_rotate_left_32(uint32_t X, uint32_t N);
uint32_t    ft_rotate_right_32(uint32_t X, uint32_t N);
uint64_t    ft_rotate_left_64(uint64_t X, uint64_t N);
uint64_t    ft_rotate_right_64(uint64_t X, uint64_t N);

# ifndef FT_PRINTF_H
#  define FT_PRINTF_H

void    ft_put_space(va_list args, int *len, int *i);
void    ft_put_plus(va_list args, int *len, int *i);
void    ft_put_sharp(va_list args, int *len, int *i, char c);
void    ft_put_zero(va_list args, int *len, int *i, char *format);
int     ft_putchar(char c);
int     ft_putstr(char *s);
int     ft_putnbr(long long n, int is_signed);
int     ft_puthex(unsigned long int n, int is_upper);
int     ft_putptr(void *p);
int     ft_printf(const char *format, ...);

# endif // FT_PRINTF_H

# ifndef FT_FPRINTF_H
#  define FT_FPRINTF_H

extern int	g_strlen;

void    ft_put_space_fd(va_list args, int *i, int fd);
void    ft_put_plus_fd(va_list args, int *i, int fd);
void    ft_put_sharp_fd(va_list args, int *i, char c, int fd);
void    ft_putchar_fd(char c, int fd);
void    ft_putstr_fd(char *s, int fd);
void    ft_putnbr_fd(long long n, int is_signed, int fd);
void    ft_puthex_fd(unsigned long int n, int is_upper, int fd);
void    ft_putptr_fd(void *p, int fd);
int     ft_fprintf(int fd, const char *format, ...);

# endif // FT_FPRINTF_H

#endif // LIBFT_H
