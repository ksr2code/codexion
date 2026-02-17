/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksmailov <ksmailov@student.42heilbronn.de  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 13:43:05 by ksmailov          #+#    #+#             */
/*   Updated: 2026/02/17 14:11:10 by ksmailov         ###   ########.fr       */
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
	if (pthread_mutex_init(&sim->log_mutex, NULL) != 0)
	{
		free(sim);
		return (NULL);
	}
	return (sim);
}

void	destroy_simulation(t_sim *sim)
{
	pthread_mutex_destroy(&sim->log_mutex);
	free(sim);
}
