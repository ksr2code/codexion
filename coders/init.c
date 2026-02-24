/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksmailov <ksmailov@student.42heilbronn.de  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 13:43:05 by ksmailov          #+#    #+#             */
/*   Updated: 2026/02/22 16:21:37 by ksmailov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_sim	*init_simulation(t_config *cfg)
{
	t_sim	*sim;

	sim = malloc(sizeof(t_sim));
	if (!sim)
		return (NULL);
	sim->start_time = get_timestamp_ms();
	sim->burnout_detected = 0;
	if (pthread_mutex_init(&sim->log_mutex, NULL) != 0)
	{
		free(sim);
		return (NULL);
	}
	if (!init_resources(sim, cfg))
	{
		destroy_simulation(sim);
		return (NULL);
	}
	return (sim);
}

static int	init_dongles(t_sim *sim, t_config *cfg)
{
	int	i;

	sim->dongles = malloc(cfg->number_of_coders * sizeof(t_dongle));
	if (!sim->dongles)
		return (0);
	i = -1;
	while (++i < cfg->number_of_coders)
	{
		sim->dongles[i].id = i;
		sim->dongles[i].available = 1;
		sim->dongles[i].cooldown_until = 0;
		sim->dongles[i].queue.capacity = Q_CAPACITY;
		sim->dongles[i].queue.requests = malloc(Q_CAPACITY * sizeof(t_request));
		if (!sim->dongles[i].queue.requests)
			return (0);
		sim->dongles[i].queue.size = 0;
		sim->dongles[i].scheduler = cfg->scheduler;
		pthread_mutex_init(&sim->dongles[i].mutex, NULL);
		pthread_cond_init(&sim->dongles[i].cond, NULL);
	}
	return (1);
}

static int	init_coder_mutexes(t_sim *sim, int count)
{
	int	i;

	i = -1;
	while (++i < count)
	{
		if (pthread_mutex_init(&sim->coders[i].compile_mutex, NULL) != 0)
		{
			while (--i >= 0)
				pthread_mutex_destroy(&sim->coders[i].compile_mutex);
			return (0);
		}
	}
	return (1);
}

static int	init_coders(t_sim *sim, t_config *cfg)
{
	int	i;

	sim->coders = malloc(cfg->number_of_coders * sizeof(t_coder));
	if (!sim->coders)
		return (0);
	i = -1;
	while (++i < cfg->number_of_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].left_dongle = &sim->dongles[i];
		sim->coders[i].right_dongle = &sim->dongles[(i + 1)
			% cfg->number_of_coders];
		sim->coders[i].cfg = cfg;
		sim->coders[i].sim = sim;
		sim->coders[i].compiles_done = 0;
		sim->coders[i].last_compile_start = sim->start_time;
		sim->coders[i].alive = 1;
	}
	if (!init_coder_mutexes(sim, cfg->number_of_coders))
	{
		free(sim->coders);
		sim->coders = NULL;
		return (0);
	}
	return (1);
}

int	init_resources(t_sim *sim, t_config *cfg)
{
	sim->num_coders = cfg->number_of_coders;
	if (!init_dongles(sim, cfg) || !init_coders(sim, cfg))
		return (0);
	return (1);
}
