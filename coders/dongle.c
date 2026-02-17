/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksmailov <ksmailov@student.42heilbronn.de  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 17:43:40 by ksmailov          #+#    #+#             */
/*   Updated: 2026/02/17 21:52:26 by ksmailov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	dongle_available(t_dongle *dongle)
{
	return (get_timestamp_ms() >= dongle->cooldown_until);
}

static void	queue_request(t_dongle *dongle, t_coder *coder)
{
	t_request	req;

	req.coder = coder;
	req.arrival_time = get_timestamp_ms();
	dongle->queue.requests[dongle->queue.size] = req;
	dongle->queue.size++;
}

static t_coder	*dequeue_fifo(t_dongle *dongle)
{
	t_coder	*winner;
	int		i;

	if (dongle->queue.size == 0)
		return (NULL);
	winner = dongle->queue.requests[0].coder;
	i = 0;
	while (++i < dongle->queue.size)
		dongle->queue.requests[i - 1] = dongle->queue.requests[i];
	dongle->queue.size--;
	return (winner);
}

void	acquire_dongle(t_coder *coder, t_dongle *dongle)
{
	struct timespec	ts;

	pthread_mutex_lock(&dongle->mutex);
	queue_request(dongle, coder);
	while (1)
	{
		if (dongle->available && dongle_available(dongle)
			&& dongle->queue.size > 0
			&& dongle->queue.requests[0].coder == coder)
		{
			dequeue_fifo(dongle);
			dongle->available = 0;
			break ;
		}
		get_timeout_ts(&ts, 10);
		pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
	}
	pthread_mutex_unlock(&dongle->mutex);
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
