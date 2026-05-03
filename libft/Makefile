NAME = libft.a

SOURCES_DIR = src
HEADERS_DIR = include
OBJECTS_DIR = objs

SOURCES = ft_isalpha.c \
    ft_isdigit.c \
    ft_isalnum.c \
    ft_isascii.c \
    ft_isprint.c \
    ft_strlen.c \
    ft_memset.c \
    ft_bzero.c \
    ft_memcpy.c \
    ft_memmove.c \
    ft_strlcpy.c \
    ft_strlcat.c \
    ft_toupper.c \
    ft_tolower.c \
    ft_strchr.c \
    ft_strrchr.c \
    ft_strcmp.c \
    ft_strncmp.c \
    ft_strcasecmp.c \
    ft_strncasecmp.c \
    ft_memchr.c \
    ft_memcmp.c \
    ft_strnstr.c \
    ft_atoi.c \
    ft_atoi_base.c \
    ft_itoa_base.c \
    ft_putnbr_base.c \
    ft_calloc.c \
    ft_strdup.c \
    ft_substr.c \
    ft_strjoin.c \
    ft_strtrim.c \
    ft_split.c \
    ft_itoa.c \
    ft_strmap.c \
    ft_strmapi.c \
    ft_striteri.c \
    ft_lerp.c \
    ft_isvalid_extension.c \
    ft_inset.c \
    ft_count_words.c \
    ft_atol.c \
    ft_bswap16.c \
    ft_bswap32.c \
    ft_bswap64.c \
    ft_rotate_left_32.c \
    ft_rotate_left_64.c \
    ft_rotate_right_32.c \
    ft_rotate_right_64.c \
    ft_printf/ft_printf.c \
    ft_printf/ft_put.c \
    ft_printf/ft_put_flags.c \
    ft_fprintf/ft_fprintf.c \
    ft_fprintf/ft_put_fd.c \
    ft_fprintf/ft_put_flags_fd.c

SOURCES_BONUS = ft_lstnew_bonus.c \
    ft_lstadd_front_bonus.c \
    ft_lstsize_bonus.c \
    ft_lstlast_bonus.c \
    ft_lstadd_back_bonus.c \
    ft_lstdelone_bonus.c \
    ft_lstclear_bonus.c \
    ft_lstiter_bonus.c \
    ft_lstmap_bonus.c

OBJECTS = $(addprefix $(OBJECTS_DIR)/, $(SOURCES:.c=.o))
OBJECTS_BONUS = $(addprefix $(OBJECTS_DIR)/, $(SOURCES_BONUS:.c=.o))

CFLAGS = -Wall -Wextra -Werror
AR = ar -rcs
RM = rm -f

$(OBJECTS_DIR)/%.o: $(SOURCES_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJECTS)
	$(AR) $(NAME) $(OBJECTS)

bonus: $(OBJECTS) $(OBJECTS_BONUS)
	$(AR) $(NAME) $(OBJECTS) $(OBJECTS_BONUS)

all: $(NAME)

clean:
	$(RM) -rf $(OBJECTS_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re bonus
