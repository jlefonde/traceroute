NAME = ft_traceroute

SOURCES_DIR = src
HEADERS_DIR = include
OBJECTS_DIR = objs

SOURCES = traceroute.c

OBJECTS = $(addprefix $(OBJECTS_DIR)/, $(SOURCES:.c=.o))

CFLAGS = -I$(HEADERS_DIR) #-Wall -Wextra -Werror
LDFLAGS = -L./libft -lft -lm

all: ./libft/libft.a $(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJECTS_DIR)/%.o: $(SOURCES_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -rf $(OBJECTS_DIR)

fclean: clean
	$(RM) -f $(NAME)
	@make fclean -C ./libft

re: fclean all

./libft/libft.a:
	make bonus -C ./libft

.PHONY: all clean fclean re
