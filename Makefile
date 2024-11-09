SRC		= experiments2.c

all		: server client

server	: $(SRC)
		cc -Wall -Wextra -Werror -D SERVER=1 -g -D CLIENT=0 -o server $(SRC) $(INCL)
client	: $(SRC)
		cc -Wall -Wextra -Werror -D SERVER=0 -g -D CLIENT=1 -o client $(SRC) $(INCL)

test	: all
		gnome-terminal -t minitalk --geometry=125x10 -- sh -c '$(shell pwd)/server; sleep 60'
