SRC		= experiments.c

all		: server client

server	: $(SRC)
		cc -Wall -Wextra -Werror -O3 -D SERVER=1 -D CLIENT=0 -o server $(SRC) $(INCL)
client	: $(SRC)
		cc -Wall -Wextra -Werror -D SERVER=0 -D CLIENT=1 -o client $(SRC) $(INCL)

TEST	: all
		gnome-terminal -t minitalk -- $(shell pwd)/server
