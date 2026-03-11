/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksmailov <ksmailov@student.42heilbronn.de  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 17:43:40 by ksmailov          #+#    #+#             */
/*   Updated: 2026/02/22 13:53:04 by ksmailov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	coder_can_compile(t_coder *coder)
{
	t_sim	*sim;
	long	now;

	sim = coder->sim;
	if (coder->left_dongle == coder->right_dongle)
		return (0);
	if (sim->queue.size == 0 || sim->queue.requests[0].coder != coder)
		return (0);
	now = get_timestamp_ms();
	return (coder->left_dongle->available
		&& now >= coder->left_dongle->cooldown_until
		&& coder->right_dongle->available
		&& now >= coder->right_dongle->cooldown_until);
}

void	acquire_both_dongles(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->pair_mutex);
	heap_insert(&sim->queue, coder, sim->scheduler);
	while (!sim->burnout)
	{
		if (coder_can_compile(coder))
		{
			coder->left_dongle->available = 0;
			coder->right_dongle->available = 0;
			heap_remove(&sim->queue);
			if (!sim->burnout)
			{
				log_state(sim, coder->id, "has taken a dongle");
				log_state(sim, coder->id, "has taken a dongle");
			}
			pthread_mutex_unlock(&sim->pair_mutex);
			return ;
		}
		pthread_cond_wait(&sim->pair_cond, &sim->pair_mutex);
	}
	pthread_mutex_unlock(&sim->pair_mutex);
}

void	release_dongles(t_coder *coder)
{
	t_sim	*sim;
	long	now;

	sim = coder->sim;
	now = get_timestamp_ms();
	pthread_mutex_lock(&sim->pair_mutex);
	coder->left_dongle->available = 1;
	coder->left_dongle->cooldown_until = now + coder->cfg->dongle_cooldown;
	coder->right_dongle->available = 1;
	coder->right_dongle->cooldown_until = now + coder->cfg->dongle_cooldown;
	pthread_cond_broadcast(&sim->pair_cond);
	pthread_mutex_unlock(&sim->pair_mutex);
}
