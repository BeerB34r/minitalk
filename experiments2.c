/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   experiments2.c                                      :+:    :+:           */
/*                                                      +:+                   */
/*   By: mde-beer <marvin@42.fr>                       +#+                    */
/*                                                    +#+                     */
/*   Created: 2024/11/07 17:50:10 by mde-beer       #+#    #+#                */
/*   Updated: 2024/11/07 19:57:28 by mde-beer       ########   odam.nl        */
/*                                                                            */
/* ************************************************************************** */

#include <signal.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#if SERVER == 1
//volatile _Atomic bool	g_bit = false;
#else
//volatile _Atomic int	g_dat = 0;
#endif

typedef struct s_packet
{
	unsigned int	sigcount;
	bool			size_known;
	uint32_t		size;
	char			*packet;
}	t_packet;

void	get_size(const bool bit, t_packet *packet, siginfo_t *signal)
{
	packet->sigcount += 1;
	packet->size <<= 1;
	packet->size += bit;
	if (packet->sigcount < 32)
	{
		kill(signal->si_pid, SIGUSR1);
		return ;
	}
	packet->size_known = true;
	packet->sigcount = 0;
	packet->packet = malloc(sizeof(char) * packet->size);
	if (!packet->packet)
	{
		*packet = (t_packet){0};
		kill(signal->si_pid, SIGUSR2);
		write(2, "Malloc error\n", 13);
		exit(1);
	}
	printf("size = %d\n", packet->size);
	kill(signal->si_pid, SIGUSR1);
}

void	server_handler(int signum, siginfo_t *signal, void *data)
{
	static t_packet	packet = {0};
	const bool		bit = (signum == SIGUSR2);
	const char		testing = 0x30 + bit;

	(void)data;
	write(1, &testing, 1);
	if (!packet.size_known)
		get_size(bit, &packet, signal);
	else if (packet.sigcount / 8 < packet.size)
	{
		packet.packet[packet.sigcount / 8] <<= 1;
		packet.packet[packet.sigcount / 8] += bit;
		packet.sigcount++;
		kill(signal->si_pid, SIGUSR1);
	}
	else
	{
		write(1, packet.packet, packet.size);
		free(packet.packet);
		packet = (t_packet){0};
		kill(signal->si_pid, SIGUSR2);
	}
}

void	client_handler(int signum, siginfo_t *signal, void *data)
{
	(void)data;
	(void)signal;
	if (signum == SIGUSR1)
		return ;
	if (signum == SIGUSR2)
		exit((write(1, "Communication ended\n", 20), 0));
}

void	send_bit(int pid, uint32_t val, size_t bit)
{
	const size_t	mask = 1 << bit;
	const bool		sign = val & mask;

	if (sign)
		kill(pid, SIGUSR2);
	else
		kill(pid, SIGUSR1);
	pause();
}

void	send_message(int pid, char *packet, uint32_t packet_size)
{
	int	i;

	i = -1;
	while (packet_size)
	{
		while (++i < 32)
			send_bit(pid, packet_size, 31 - i);
	}
	while (*packet)
	{
		i = -1;
		while (++i < 8)
			send_bit(pid, (unsigned char)*packet, 7 - i);
		packet++;
	}
	kill(pid, SIGUSR1);
}

int	main(int argc, char **argv)
{
	struct sigaction	handler;

	sigemptyset(&handler.sa_mask);
	sigaddset(&handler.sa_mask, SIGUSR1);
	sigaddset(&handler.sa_mask, SIGUSR2);
	if (SERVER)
	{
		printf("PID: %d\n", getpid());
		handler.sa_sigaction = server_handler;
		handler.sa_flags = (SA_SIGINFO);
		sigaction(SIGUSR1, &handler, NULL);
		sigaction(SIGUSR2, &handler, NULL);
		while (1)
			pause();
	}
	else
	{
		if (argc < 3)
			return (printf("<usage> %s [server PID] [packet]", argv[0]), 0);
		handler.sa_sigaction = client_handler;
		handler.sa_flags = (SA_SIGINFO);
		sigaction(SIGUSR1, &handler, NULL);
		sigaction(SIGUSR2, &handler, NULL);
		send_message(atoi(argv[1]), argv[2], strlen(argv[2]));
		pause();
	}
}
