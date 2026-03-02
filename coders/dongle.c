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

static int	dongle_available(t_dongle *dongle, t_coder *coder)
{
	int	result;

	pthread_mutex_lock(&dongle->mutex);
	result = (dongle->available && get_timestamp_ms() >= dongle->cooldown_until
			&& dongle->queue.size > 0
			&& dongle->queue.requests[0].coder == coder);
	pthread_mutex_unlock(&dongle->mutex);
	return (result);
}

static void	add_to_queue(t_coder *coder, t_dongle *first, t_dongle *second)
{
	pthread_mutex_lock(&first->mutex);
	heap_insert(first, coder);
	pthread_mutex_unlock(&first->mutex);
	pthread_mutex_lock(&second->mutex);
	heap_insert(second, coder);
	pthread_mutex_unlock(&second->mutex);
}

static void	get_dongles(t_dongle *first, t_dongle *second)
{
	pthread_mutex_lock(&first->mutex);
	heap_remove(first);
	first->available = 0;
	pthread_mutex_unlock(&first->mutex);
	pthread_mutex_lock(&second->mutex);
	heap_remove(second);
	second->available = 0;
	pthread_mutex_unlock(&second->mutex);
}

void	acquire_both_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	get_ordered(coder, &first, &second);
	pthread_mutex_lock(&coder->sim->pair_mutex);
	add_to_queue(coder, first, second);
	while (!coder->sim->burnout)
	{
		if (dongle_available(first, coder) && dongle_available(second, coder))
		{
			get_dongles(first, second);
			if (!coder->sim->burnout)
			{
				log_state(coder->sim, coder->id, "has taken a dongle");
				log_state(coder->sim, coder->id, "has taken a dongle");
			}
			pthread_mutex_unlock(&coder->sim->pair_mutex);
			return ;
		}
		pthread_mutex_unlock(&coder->sim->pair_mutex);
		usleep(1000);
		pthread_mutex_lock(&coder->sim->pair_mutex);
	}
	pthread_mutex_unlock(&coder->sim->pair_mutex);
}

void	release_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;
	long		now;

	now = get_timestamp_ms();
	get_ordered(coder, &first, &second);
	pthread_mutex_lock(&first->mutex);
	first->available = 1;
	first->cooldown_until = now + coder->cfg->dongle_cooldown;
	pthread_mutex_unlock(&first->mutex);
	pthread_mutex_lock(&second->mutex);
	second->available = 1;
	second->cooldown_until = now + coder->cfg->dongle_cooldown;
	pthread_mutex_unlock(&second->mutex);
}
