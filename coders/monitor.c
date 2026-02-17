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

static void	wake_all_dongles(t_sim *sim)
{
	int	i;

	i = -1;
	while (++i < sim->num_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
	}
}

static int	check_coder_burnout(t_sim *sim, t_coder *coder)
{
	long	elapsed;

	if (!coder->alive)
		return (0);
	elapsed = get_timestamp_ms() - coder->last_compile_start;
	if (elapsed > coder->cfg->time_to_burnout)
	{
		log_burnout(sim, coder->id);
		sim->burnout_detected = 1;
		wake_all_dongles(sim);
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
		usleep(10000);
		i = -1;
		while (++i < sim->num_coders)
		{
			if (check_coder_burnout(sim, &sim->coders[i]))
				return (NULL);
		}
	}
	return (NULL);
}
