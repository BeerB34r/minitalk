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

void	server_handler(int signum, siginfo_t *signal, void *data)
{
	static uint32_t	sigcount	= 0;
	static uint32_t	packet_size = 0;
	static bool		size_recieved = false;
	static char 	*packet = NULL;

	(void)data;
	if (size_recieved == false)
	{
		packet_size <<= 1;
		packet_size += (signum == SIGUSR2);
		if (++sigcount < 32)
			return ;
		size_recieved = true;
		packet = calloc(packet_size + 1, sizeof(char));
		if (!packet)
			return ;
	}
	else
	{
		packet[(sigcount - 32) / 8] <<= 1;
		packet[(sigcount - 32) / 8] += (signum == SIGUSR2);
		if (((++sigcount - 32) / 8) != packet_size)
			return ;
		printf("%s\n", packet);
		free(packet);
		packet = NULL;
		size_recieved = false;
		packet_size = 0;
		sigcount = 0;
		kill(signal->si_pid, SIGUSR1);
	}
}

void	client_handler(int sig)
{
	(void)sig;
	write(1, "Message successfully delivered\n", 31);
	exit(0);
}

#define SLP_TIME 200

static int	send_bit(int bit)
{
	if (bit)
		return (SIGUSR2);
	return (SIGUSR1);
}

void	send_message(int pid, char *packet)
{
	int packet_size;
	int	i;
	int	j;

	packet_size = strlen(packet);
	i = -1;
	while (++i < 32)
	{
		kill(pid, send_bit((packet_size >> (31 - i)) & 1));
		usleep(SLP_TIME);
	}
	i = -1;
	while (packet[++i])
	{
		j = -1;
		while (++j < 8)
		{
			kill(pid, send_bit(((packet[i] >> (7 - j)) & 1)));
			usleep(SLP_TIME);
		}
	}
	if (packet_size == 0)
		kill(pid, SIGUSR1);
}

int	main(int argc, char **argv)
{
	pid_t 				pid;
	struct sigaction	sig_act;
	
	if (SERVER)
	{
		pid = getpid();
		printf("pid: %d\n", pid);
		sig_act.sa_sigaction = &server_handler;
		sig_act.sa_flags = (SA_SIGINFO);
		sigaction(SIGUSR1, &sig_act, NULL);
		sigaction(SIGUSR2, &sig_act, NULL);
		while (1)
			pause();
	}
	else if (CLIENT)
	{
		if (argc < 3)
			return (printf("<USAGE> %s [server PID] [packet]", argv[0]));
		signal(SIGUSR1, client_handler);
		signal(SIGUSR2, client_handler);
		printf("%ld %s\n",strlen(argv[2]), argv[2]);
		send_message(atoi(argv[1]), argv[2]);
		pause();
	}
}
