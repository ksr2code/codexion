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
	t_dongle		*first;
	t_dongle		*second;
	struct timespec	ts;

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
		get_timeout_ts(&ts, 10);
		pthread_cond_timedwait(&coder->sim->pair_cond, &coder->sim->pair_mutex,
			&ts);
	}
	pthread_mutex_unlock(&coder->sim->pair_mutex);
}

void	release_dongles(t_coder *coder)
{
	long	now;

	now = get_timestamp_ms();
	pthread_mutex_lock(&coder->left_dongle->mutex);
	coder->left_dongle->available = 1;
	coder->left_dongle->cooldown_until = now + coder->cfg->dongle_cooldown;
	pthread_mutex_unlock(&coder->left_dongle->mutex);
	pthread_mutex_lock(&coder->right_dongle->mutex);
	coder->right_dongle->available = 1;
	coder->right_dongle->cooldown_until = now + coder->cfg->dongle_cooldown;
	pthread_mutex_unlock(&coder->right_dongle->mutex);
	pthread_mutex_lock(&coder->sim->pair_mutex);
	pthread_cond_broadcast(&coder->sim->pair_cond);
	pthread_mutex_unlock(&coder->sim->pair_mutex);
}
