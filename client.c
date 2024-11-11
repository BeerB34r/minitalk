#include <signal.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <libft.h>

volatile bool	g_msg_recieved = 0;

void	send_bit(int pid, uint32_t val, uint32_t bit)
{
	const uint32_t	mask = 1 << bit;
	const bool		sign = val & mask;

	if (sign)
		kill(pid, SIGUSR2);
	else
		kill(pid, SIGUSR1);
	while (!g_msg_recieved)
		usleep(1);
	g_msg_recieved = 0;
}

void	send_message(int pid, char *packet, uint32_t packet_size)
{
	int	i;

	i = -1;
	while (++i < 32)
		send_bit(pid, packet_size, 31 - i);
	while (*packet)
	{
		i = -1;
		while (++i < 8)
			send_bit(pid, (unsigned char)*packet, 7 - i);
		packet++;
	}
}

void	client_handler(int signum, siginfo_t *signal, void *context)
{
	(void)context;
	(void)signal;
	if (signum == SIGUSR1)
		g_msg_recieved = 1;
	else
		exit((write(1, "Communication ended\n", 20), 0));
}

int	main(int argc, char **argv)
{
	struct sigaction	handler;
	int					server_pid;

	sigemptyset(&handler.sa_mask);
	sigaddset(&handler.sa_mask, SIGUSR1);
	sigaddset(&handler.sa_mask, SIGUSR2);
	if (argc < 3)
		return (ft_printf("Usage: %s [server PID] [packet]", argv[0]), 0);
	server_pid = ft_atoi(argv[1]);
	if (server_pid < 1)
		return (ft_printf("Usage: %s [server PID] [packet]", argv[0]), 0);
	handler.sa_sigaction = client_handler;
	handler.sa_flags = (SA_SIGINFO | SA_RESTART);
	sigaction(SIGUSR1, &handler, NULL);
	sigaction(SIGUSR2, &handler, NULL);
	send_message(server_pid, argv[2], strlen(argv[2]));
	while (1)
		pause();
}
