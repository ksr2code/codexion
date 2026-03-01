/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksmailov <ksmailov@student.42heilbronn.de  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 21:29:45 by ksmailov          #+#    #+#             */
/*   Updated: 2026/02/17 22:04:21 by ksmailov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	check_coder_burnout(t_sim *sim, t_coder *coder)
{
	long	elapsed;

	pthread_mutex_lock(&coder->compile_mutex);
	if (!coder->alive)
	{
		pthread_mutex_unlock(&coder->compile_mutex);
		return (0);
	}
	elapsed = get_timestamp_ms() - coder->last_compile_start;
	pthread_mutex_unlock(&coder->compile_mutex);
	if (elapsed >= coder->cfg->time_to_burnout)
	{
		pthread_mutex_lock(&sim->pair_mutex);
		sim->burnout = 1;
		pthread_cond_broadcast(&sim->burnout_cond);
		pthread_mutex_unlock(&sim->pair_mutex);
		log_burnout(sim, coder->id);
		return (1);
	}
	return (0);
}

int	burnout_detected(t_sim *sim)
{
	int	val;

	pthread_mutex_lock(&sim->pair_mutex);
	val = sim->burnout;
	pthread_mutex_unlock(&sim->pair_mutex);
	return (val);
}

void	*monitor_routine(void *data)
{
	t_sim	*sim;
	int		i;

	sim = (t_sim *)data;
	while (!burnout_detected(sim))
	{
		usleep(1000);
		i = -1;
		while (++i < sim->num_coders)
			if (check_coder_burnout(sim, &sim->coders[i]))
				return (NULL);
	}
	return (NULL);
}

int	create_monitor(t_sim *sim)
{
	if (pthread_create(&sim->monitor_thread, NULL, monitor_routine, sim) != 0)
	{
		pthread_mutex_lock(&sim->pair_mutex);
		sim->burnout = 1;
		pthread_cond_broadcast(&sim->burnout_cond);
		pthread_mutex_unlock(&sim->pair_mutex);
		wait_coders(sim);
		return (0);
	}
	return (1);
}

void	wait_monitor(t_sim *sim)
{
	pthread_mutex_lock(&sim->pair_mutex);
	sim->burnout = 1;
	pthread_cond_broadcast(&sim->burnout_cond);
	pthread_mutex_unlock(&sim->pair_mutex);
	pthread_join(sim->monitor_thread, NULL);
}
