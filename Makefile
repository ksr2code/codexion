NAME = codexion

SRC_DIR = coders
SRC = $(SRC_DIR)/main.c \
       $(SRC_DIR)/clean.c \
       $(SRC_DIR)/coder.c \
       $(SRC_DIR)/coder_utils.c \
       $(SRC_DIR)/dongle.c \
       $(SRC_DIR)/init.c \
       $(SRC_DIR)/logger.c \
       $(SRC_DIR)/monitor.c \
       $(SRC_DIR)/parse.c \
       $(SRC_DIR)/time_utils.c

OBJ = $(SRC:.c=.o)

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
