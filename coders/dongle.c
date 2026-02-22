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

static int	dongle_available(t_dongle *dongle)
{
	return (get_timestamp_ms() >= dongle->cooldown_until);
}

void	acquire_dongle(t_coder *coder, t_dongle *dongle)
{
	struct timespec	ts;

	pthread_mutex_lock(&dongle->mutex);
	heap_insert(dongle, coder);
	while (!coder->sim->burnout_detected)
	{
		if (dongle->available && dongle_available(dongle)
			&& dongle->queue.size > 0
			&& dongle->queue.requests[0].coder == coder)
		{
			heap_remove(dongle);
			dongle->available = 0;
			break ;
		}
		get_timeout_ts(&ts, 10);
		pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
	}
	pthread_mutex_unlock(&dongle->mutex);
	if (!coder->sim->burnout_detected)
		log_state(coder->sim, coder->id, "has taken a dongle");
}

void	release_dongles(t_coder *coder)
{
	long	now;

	now = get_timestamp_ms();
	pthread_mutex_lock(&coder->left_dongle->mutex);
	coder->left_dongle->available = 1;
	coder->left_dongle->cooldown_until = now + coder->cfg->dongle_cooldown;
	pthread_cond_broadcast(&coder->left_dongle->cond);
	pthread_mutex_unlock(&coder->left_dongle->mutex);
	pthread_mutex_lock(&coder->right_dongle->mutex);
	coder->right_dongle->available = 1;
	coder->right_dongle->cooldown_until = now + coder->cfg->dongle_cooldown;
	pthread_cond_broadcast(&coder->right_dongle->cond);
	pthread_mutex_unlock(&coder->right_dongle->mutex);
}
