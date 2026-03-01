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

static int	init_sim_mutex_cond(t_sim *sim)
{
	sim->is_init = 0;
	if (pthread_mutex_init(&sim->log_mutex, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&sim->pair_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->log_mutex);
		return (0);
	}
	if (pthread_cond_init(&sim->pair_cond, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->log_mutex);
		pthread_mutex_destroy(&sim->pair_mutex);
		return (0);
	}
	sim->is_init = 1;
	return (1);
}

t_sim	*init_simulation(t_config *cfg)
{
	t_sim	*sim;

	sim = malloc(sizeof(t_sim));
	if (!sim)
		return (NULL);
	memset(sim, 0, sizeof(t_sim));
	sim->start_time = get_timestamp_ms();
	sim->burnout = 0;
	if (!init_sim_mutex_cond(sim) || !init_resources(sim, cfg))
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
	memset(sim->dongles, 0, cfg->number_of_coders * sizeof(t_dongle));
	i = -1;
	while (++i < cfg->number_of_coders)
	{
		sim->dongles[i].id = i;
		sim->dongles[i].available = 1;
		sim->dongles[i].cooldown_until = 0;
		sim->dongles[i].queue.requests = malloc(2 * sizeof(t_request));
		if (!sim->dongles[i].queue.requests)
			return (0);
		sim->dongles[i].queue.size = 0;
		sim->dongles[i].scheduler = cfg->scheduler;
		if (pthread_mutex_init(&sim->dongles[i].mutex, NULL) != 0)
			return (sim->dongles[i].is_init = 0, 0);
		sim->dongles->is_init = 1;
	}
	return (1);
}

static int	init_coders(t_sim *sim, t_config *cfg)
{
	int	i;

	sim->coders = malloc(cfg->number_of_coders * sizeof(t_coder));
	if (!sim->coders)
		return (0);
	memset(sim->coders, 0, cfg->number_of_coders * sizeof(t_coder));
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
		if (pthread_mutex_init(&sim->coders[i].compile_mutex, NULL) != 0)
			return (sim->coders[i].is_init = 0, 0);
		sim->coders[i].is_init = 1;
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
