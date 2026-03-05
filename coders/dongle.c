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

static int	dongle_available_unlocked(t_dongle *dongle, t_coder *coder)
{
	return (dongle->available && get_timestamp_ms() >= dongle->cooldown_until
			&& dongle->queue.size > 0
			&& dongle->queue.requests[0].coder == coder);
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

void	acquire_both_dongles(t_coder *coder)
{
	t_dongle		*first;
	t_dongle		*second;
	struct timespec	ts;
	int			first_ready;
	int			second_ready;

	get_ordered(coder, &first, &second);
	pthread_mutex_lock(&coder->sim->pair_mutex);
	add_to_queue(coder, first, second);
	pthread_mutex_unlock(&coder->sim->pair_mutex);
	while (!burnout_detected(coder->sim))
	{
		first_ready = 0;
		pthread_mutex_lock(&first->mutex);
		if (dongle_available_unlocked(first, coder))
		{
			pthread_mutex_unlock(&first->mutex);
			first_ready = 1;
		}
		else
		{
			get_timeout_ts(&ts, 10);
			pthread_cond_timedwait(&first->cond, &first->mutex, &ts);
			pthread_mutex_unlock(&first->mutex);
			continue;
		}
		second_ready = 0;
		pthread_mutex_lock(&second->mutex);
		if (dongle_available_unlocked(second, coder))
		{
			pthread_mutex_unlock(&second->mutex);
			second_ready = 1;
		}
		else
		{
			get_timeout_ts(&ts, 10);
			pthread_cond_timedwait(&second->cond, &second->mutex, &ts);
			pthread_mutex_unlock(&second->mutex);
			continue;
		}
		if (first_ready && second_ready)
		{
			pthread_mutex_lock(&coder->sim->pair_mutex);
			pthread_mutex_lock(&first->mutex);
			pthread_mutex_lock(&second->mutex);
			if (dongle_available_unlocked(first, coder)
				&& dongle_available_unlocked(second, coder))
			{
				heap_remove(first);
				heap_remove(second);
				first->available = 0;
				second->available = 0;
				if (!coder->sim->burnout)
				{
					log_state(coder->sim, coder->id, "has taken a dongle");
					log_state(coder->sim, coder->id, "has taken a dongle");
				}
				pthread_mutex_unlock(&second->mutex);
				pthread_mutex_unlock(&first->mutex);
				pthread_mutex_unlock(&coder->sim->pair_mutex);
				return ;
			}
			pthread_mutex_unlock(&second->mutex);
			pthread_mutex_unlock(&first->mutex);
			pthread_mutex_unlock(&coder->sim->pair_mutex);
		}
	}
}

void	release_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;
	long		now;

	now = get_timestamp_ms();
	get_ordered(coder, &first, &second);
	pthread_mutex_lock(&coder->sim->pair_mutex);
	pthread_mutex_lock(&first->mutex);
	pthread_mutex_lock(&second->mutex);
	first->available = 1;
	first->cooldown_until = now + coder->cfg->dongle_cooldown;
	pthread_cond_broadcast(&first->cond);
	pthread_mutex_unlock(&first->mutex);
	second->available = 1;
	second->cooldown_until = now + coder->cfg->dongle_cooldown;
	pthread_cond_broadcast(&second->cond);
	pthread_mutex_unlock(&second->mutex);
	pthread_mutex_unlock(&coder->sim->pair_mutex);
}
