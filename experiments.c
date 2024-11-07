/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   experiments.c                                       :+:    :+:           */
/*                                                      +:+                   */
/*   By: mde-beer <marvin@42.fr>                       +#+                    */
/*                                                    +#+                     */
/*   Created: 2024/11/07 11:03:24 by mde-beer       #+#    #+#                */
/*   Updated: 2024/11/07 17:47:38 by mde-beer       ########   odam.nl        */
/*                                                                            */
/* ************************************************************************** */

//#include <minitalk.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <wait.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct s_packet
{
	uint32_t	sigcount;
	uint32_t	size;
	bool		size_recieved;
	char		*content;
}	t_packet;

#define SLP_TIME 1

volatile _Atomic bool	g_bit;

void	server_handler(int signum, siginfo_t *signal, void *data)
{
	static t_packet					packet = {0};
	char							c;

	(void)data;
	g_bit = (signum == SIGUSR2);
	c = 0x30 + g_bit;
	if (signum == SIGINT)
	{
		packet = (t_packet){0};
		write(2, "\npacket reset\n", 14);
		return ;
	}
	if (packet.sigcount == 0)
		write(2, "\nrecieving message...\n", 22);
	write(2, &c, 1);
	if (packet.size_recieved == false)
	{
		packet.size <<= 1;
		packet.size += g_bit;
		kill(signal->si_pid, SIGUSR2);
		if (++packet.sigcount < 32)
			return ;
		packet.size_recieved = true;
		write(2, "|", 1);
		packet.content = calloc(packet.size + 1, sizeof(char));
		if (!packet.content)
			return ;
	}
	else
	{
		packet.content[(packet.sigcount - 32) / 8] <<= 1;
		packet.content[(packet.sigcount - 32) / 8] += g_bit;
		usleep(1);
		kill(signal->si_pid, SIGUSR2);
		if (((++packet.sigcount - 32) / 8) < packet.size)
			return ;
		write(1, "\n", 1);
		write(1, packet.content, packet.size);
		free(packet.content);
		packet = (t_packet){0};
		kill(signal->si_pid, SIGUSR1);
		return ;
	}
}

void	client_handler(int sig)
{
	g_bit = (sig == SIGUSR2);
	if (g_bit)
		return ;
	write(1, "\nMessage successfully delivered\n", 32);
	exit(0);
}

static int	send_info(int info)
{
	if (info)
		return (SIGUSR2);
	return (SIGUSR1);
}

void	send_message(int pid, char *packet)
{
	int		packet_size;
	int		i;
	int		j;
	char	c;

	packet_size = strlen(packet);
	i = -1;
	while (++i < 32)
	{
		kill(pid, send_info((packet_size >> (31 - i)) & 1));
		pause();
		c = 0x30 + ((packet_size >> (31 - i)) & 1);
		write(2, &c, 1);
	}
	i = -1;
	write(2, "|", 1);
	while (packet[++i])
	{
		j = -1;
		while (++j < 8)
		{
			kill(pid, send_info(((packet[i] >> (7 - j)) & 1)));
			pause();
			c = 0x30 + ((packet[i] >> (7 - j)) & 1);
			write(2, &c, 1);
		}
	}
	if (packet_size == 0)
		kill(pid, SIGUSR1);
}

int	main(int argc, char **argv)
{
	pid_t				pid;
	struct sigaction	sig_act;

	if (SERVER)
	{
		pid = getpid();
		printf("pid: %d\n", pid);
		sig_act.sa_sigaction = &server_handler;
		sig_act.sa_flags = (SA_SIGINFO | SA_RESTART);
		sigaction(SIGUSR1, &sig_act, NULL);
		sigaction(SIGUSR2, &sig_act, NULL);
		sigaction(SIGINT, &sig_act, NULL);
		while (1)
			pause();
	}
	else if (CLIENT)
	{
		if (argc < 3)
			return (printf("<USAGE> %s [server PID] [packet]", argv[0]));
		signal(SIGUSR1, client_handler);
		signal(SIGUSR2, client_handler);
		send_message(atoi(argv[1]), argv[2]);
		pause();
	}
}
