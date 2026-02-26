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

	if (!coder->alive)
		return (0);
	pthread_mutex_lock(&coder->compile_mutex);
	elapsed = get_timestamp_ms() - coder->last_compile_start;
	pthread_mutex_unlock(&coder->compile_mutex);
	if (elapsed >= coder->cfg->time_to_burnout)
	{
		pthread_mutex_lock(&sim->pair_mutex);
		sim->burnout_detected = 1;
		pthread_cond_broadcast(&sim->pair_cond);
		pthread_mutex_unlock(&sim->pair_mutex);
		log_burnout(sim, coder->id);
		return (1);
	}
	return (0);
}

void	*monitor_routine(void *data)
{
	t_sim	*sim;
	int		i;

	sim = (t_sim *)data;
	while (!sim->burnout_detected)
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
		sim->burnout_detected = 1;
		wait_coders(sim);
		return (0);
	}
	return (1);
}

void	wait_monitor(t_sim *sim)
{
	sim->burnout_detected = 1;
	pthread_join(sim->monitor_thread, NULL);
}
