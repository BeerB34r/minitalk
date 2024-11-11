/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   server.c                                            :+:    :+:           */
/*                                                      +:+                   */
/*   By: mde-beer <marvin@42.fr>                       +#+                    */
/*                                                    +#+                     */
/*   Created: 2024/11/11 11:44:48 by mde-beer       #+#    #+#                */
/*   Updated: 2024/11/11 11:47:47 by mde-beer       ########   odam.nl        */
/*                                                                            */
/* ************************************************************************** */

#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <libft.h>
#include "minitalk.h"

volatile int	g_msg_recieved[3] = {0};

void	mt_get_size(const t_bool bit, t_packet *packet, int sender)
{
	packet->size = (packet->size << 1) | bit;
	if (packet->sigcount == 32)
	{
		packet->size_known = truey;
		packet->sigcount = 0;
	}
	kill(sender, SIGUSR1);
}

void	mt_get_char(const t_bool bit, t_packet *packet, int sender)
{
	int	signal;

	packet->c = (packet->c << 1) | bit;
	if (!(packet->sigcount % 8))
		write(1, &(packet->c), 1);
	signal = SIGUSR1;
	if (packet->sigcount / 8 == packet->size)
	{
		*packet = (t_packet){0};
		write(1, "\n", 1);
		signal = SIGUSR2;
	}
	kill(sender, signal);
}

void	message_handler(const t_bool bit, int sender_pid)
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
		mt_get_size(bit, &packet, sender);
	else
		mt_get_char(bit, &packet, sender);
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
	ft_printf("PID: %d\n", getpid());
	handler.sa_sigaction = server_handler;
	handler.sa_flags = (SA_SIGINFO);
	sigaction(SIGUSR1, &handler, NULL);
	sigaction(SIGUSR2, &handler, NULL);
	while (truey)
	{
		while (!g_msg_recieved[0])
			usleep(1);
		g_msg_recieved[0] = 0;
		message_handler(g_msg_recieved[1], g_msg_recieved[2]);
	}
}
