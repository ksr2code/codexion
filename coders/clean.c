/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   clean.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksmailov <ksmailov@student.42heilbronn.de  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 14:47:34 by ksmailov          #+#    #+#             */
/*   Updated: 2026/02/22 16:51:23 by ksmailov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	destroy_dongles(t_sim *sim)
{
	int	i;

	i = -1;
	while (++i < sim->num_coders)
	{
		free(sim->dongles[i].queue.requests);
		if (sim->dongles[i].is_init)
			pthread_mutex_destroy(&sim->dongles[i].mutex);
	}
	free(sim->dongles);
}

static void	destroy_coders(t_sim *sim)
{
	int	i;

	i = -1;
	while (++i < sim->num_coders)
		if (sim->coders[i].is_init)
			pthread_mutex_destroy(&sim->coders[i].compile_mutex);
	free(sim->coders);
}

void	destroy_simulation(t_sim *sim)
{
	if (sim->dongles)
		destroy_dongles(sim);
	if (sim->coders)
		destroy_coders(sim);
	if (sim->is_init)
	{
		pthread_mutex_destroy(&sim->log_mutex);
		pthread_mutex_destroy(&sim->pair_mutex);
		pthread_cond_destroy(&sim->pair_cond);
	}
	free(sim);
}
