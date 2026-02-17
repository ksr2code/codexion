/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksmailov <ksmailov@student.42heilbronn.de  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 15:02:23 by ksmailov          #+#    #+#             */
/*   Updated: 2026/02/17 19:46:03 by ksmailov         ###   ########.fr       */
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

static void	acquire_lower_first(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	if (coder->left_dongle->id < coder->right_dongle->id)
	{
		first = coder->left_dongle;
		second = coder->right_dongle;
	}
	else
	{
		first = coder->right_dongle;
		second = coder->left_dongle;
	}
	acquire_dongle(coder, first);
	acquire_dongle(coder, second);
}

void	*coder_routine(void *data)
{
	t_coder	*coder;

	coder = (t_coder *)data;
	while (coder->compiles_done < coder->cfg->number_of_compiles_required
		&& coder->alive)
	{
		acquire_lower_first(coder);
		coder->last_compile_start = get_timestamp_ms();
		coder->compiles_done++;
		log_state(coder->sim, coder->id, "is compiling");
		msleep(coder->cfg->time_to_compile);
		log_state(coder->sim, coder->id, "is debugging");
		msleep(coder->cfg->time_to_debug);
		log_state(coder->sim, coder->id, "is refactoring");
		msleep(coder->cfg->time_to_refactor);
		release_dongles(coder);
	}
	coder->alive = 0;
	return (NULL);
}
