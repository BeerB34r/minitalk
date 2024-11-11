/* ************************************************************************** */
/*                                                                            */
/*                                                         ::::::::           */
/*   minitalk.h                                          :+:    :+:           */
/*                                                      +:+                   */
/*   By: mde-beer <marvin@42.fr>                       +#+                    */
/*                                                    +#+                     */
/*   Created: 2024/11/11 11:42:50 by mde-beer       #+#    #+#                */
/*   Updated: 2024/11/11 11:47:50 by mde-beer       ########   odam.nl        */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINITALK_H
# define MINITALK_H
# include <stdint.h>

typedef struct s_packet
{
	unsigned int	sigcount;
	t_bool			size_known;
	uint32_t		size;
	char			c;
}	t_packet;

#endif
