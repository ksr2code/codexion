/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksmailov <ksmailov@student.42heilbronn.de  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 13:34:42 by ksmailov          #+#    #+#             */
/*   Updated: 2026/02/17 22:07:21 by ksmailov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <pthread.h>

long	get_timestamp_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void	msleep(t_sim *sim, long ms)
{
	long	start;
	long	elapsed;
	int		burnout;

	start = get_timestamp_ms();
	burnout = 0;
	while (!burnout)
	{
		pthread_mutex_lock(&sim->pair_mutex);
		burnout = sim->burnout_detected;
		pthread_mutex_unlock(&sim->pair_mutex);
		elapsed = get_timestamp_ms() - start;
		if (elapsed >= ms)
			break ;
		if (!burnout)
			usleep(1000);
	}
}

void	get_timeout_ts(struct timespec *ts, long timeout_ms)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	ts->tv_sec = tv.tv_sec + (timeout_ms / 1000);
	ts->tv_nsec = (tv.tv_usec * 1000) + ((timeout_ms % 1000) * 1000000);
	if (ts->tv_nsec >= 1000000000)
	{
		ts->tv_sec++;
		ts->tv_nsec -= 1000000000;
	}
}
