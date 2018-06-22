NAME1 = serveur
NAME2 = client
CC = gcc
CFLAGS = -Wall -Wextra -Werror
SRC1 = tchatche_server.c list_functions.c functions.c list_trsf_functions.c
SRC2 = tchatche_client.c functions.c list_trsf_functions.c
OBJ1 = $(SRC1:.c=.o)
OBJ2 = $(SRC2:.c=.o)
all: $(NAME1) $(NAME2)

$(NAME1):
	$(CC) -c $(CFLAGS) $(SRC1)
	$(CC) -o $(NAME1) $(OBJ1)

$(NAME2):
	$(CC) -c $(CFLAGS) $(SRC2)
	$(CC) -o $(NAME2) $(OBJ2)

clean:
	rm -rf $(OBJ1)
	rm -rf $(OBJ2)

fclean: clean
	rm -rf $(NAME1)
	rm -rf $(NAME2)

re: fclean all
