/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksmailov <ksmailov@student.42heilbronn.de  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 15:02:23 by ksmailov          #+#    #+#             */
/*   Updated: 2026/02/17 15:07:23 by ksmailov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	create_coders(t_sim *sim)
{
	int	i;

	i = -1;
	while (++i < sim->num_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL, coder_routine,
				&sim->coders[i]) != 0)
			return (0);
	}
	return (1);
}

void	wait_coders(t_sim *sim)
{
	int	i;

	i = -1;
	while (++i < sim->num_coders)
		pthread_join(sim->coders[i].thread, NULL);
}

void	*coder_routine(void *data)
{
	t_coder	*coder;

	coder = (t_coder *)data;
	log_state(coder->sim, coder->id, "is compiling");
	msleep(coder->cfg->time_to_compile);
	log_state(coder->sim, coder->id, "is debugging");
	msleep(coder->cfg->time_to_debug);
	log_state(coder->sim, coder->id, "is refactoring");
	msleep(coder->cfg->time_to_refactor);
	return (NULL);
}
