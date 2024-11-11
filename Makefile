CC			= cc
CFLAGS		= -Wall -Wextra -Werror

CLIENTSRC	= client.c
SERVERSRC	= server.c
LIBFT		= libft/libft.a
INCL		= -I ./libft/include
CLIENTOBJ	= $(CLIENTSRC:.c=.o)
SERVEROBJ	= $(SERVERSRC:.c=.o)
SERVER		= server
CLIENT		= client

all			: server client

$(SERVER)	: $(SERVEROBJ) $(LIBFT)
			$(CC) $(CFLAGS) -o $@ $^ $(INCL)
$(CLIENT)	: $(CLIENTOBJ) $(LIBFT)
			$(CC) $(CFLAGS) -o $@ $^ $(INCL)
$(LIBFT)	:
			make -C ./libft
%.o			: %.c
			$(CC) $(CFLAGS) -c $^ $(INCL)

fclean		: clean
			make fclean -C ./libft
			rm -f $(CLIENT) $(SERVER)

clean		:
			make clean -C ./libft
			rm -f $(CLIENTOBJ) $(SERVEROBJ)

test		: all
			gnome-terminal -t minitalk --geometry=125x10 -- sh -c '$(shell pwd)/server; sleep 60'

.PHONY		: all test clean fclean
