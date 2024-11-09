#include <signal.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

volatile _Atomic int	g_msg_recieved[3] = {0};

typedef struct s_packet
{
	unsigned int	sigcount;
	bool			size_known;
	uint32_t		size;
	char			c;
}	t_packet;

void	get_size(const bool bit, t_packet *packet, int sender)
{
	packet->size = (packet->size << 1) | bit;
	if (packet->sigcount == 32)
	{
		packet->size_known = true;
		packet->sigcount = 0;
	}
	kill(sender, SIGUSR1);
}

void	get_char(const bool bit, t_packet *packet, int sender)
{
	packet->c = (packet->c << 1) | bit;
	if (!(packet->sigcount % 8))
		write(1, &(packet->c), 1);
	if (packet->sigcount / 8 != packet->size)
	{
		kill(sender, SIGUSR1);
		return ;
	}
	*packet = (t_packet){0};
	write(1, "\n", 1);
	kill(sender, SIGUSR2);
}

void	message_handler(const bool bit, int sender_pid)
{
	static t_packet	packet = {0};
	static int		sender = 0;

	if (sender != sender_pid)
	{
		packet = (t_packet){0};
		write(1, "\n", 1);
		sender = sender_pid;
	}
	packet.sigcount++;
	if (!packet.size_known)
		get_size(bit, &packet, sender);
	else
		get_char(bit, &packet, sender);
}

void	server_handler(int signum, siginfo_t *signal, void *context)
{
	(void)context;
	g_msg_recieved[1] = (signum == SIGUSR2);
	g_msg_recieved[2] = signal->si_pid;
	g_msg_recieved[0] = 1;
}

int	main(void)
{
	struct sigaction	handler;

	sigemptyset(&handler.sa_mask);
	sigaddset(&handler.sa_mask, SIGUSR1);
	sigaddset(&handler.sa_mask, SIGUSR2);
	printf("PID: %d\n", getpid());
	handler.sa_sigaction = server_handler;
	handler.sa_flags = (SA_SIGINFO);
	sigaction(SIGUSR1, &handler, NULL);
	sigaction(SIGUSR2, &handler, NULL);
	while (true)
	{
		while (!g_msg_recieved[0])
			pause();
		g_msg_recieved[0] = 0;
		message_handler(g_msg_recieved[1], g_msg_recieved[2]);
	}
}
