CC			= cc
CFLAGS		= -Wall -Wextra -Werror

CLIENTSRC	= client.c
SERVERSRC	= server.c
INCL		= -I ./libft/include -L ./libft -lft
CLIENTOBJ	= $(CLIENTSRC:.c=.o)
SERVEROBJ	= $(SERVERSRC:.c=.o)

all			: server client

server		: $(SERVEROBJ)
			$(CC) $(CFLAGS) -o $@ $^ $(INCL)
client		: $(CLIENTOBJ)
			$(CC) $(CFLAGS) -o $@ $^ $(INCL)
%.o			: %.c
			$(CC) $(CFLAGS) -c $^

test		: all
			gnome-terminal -t minitalk --geometry=125x10 -- sh -c '$(shell pwd)/server; sleep 60'

.PHONY		: all
