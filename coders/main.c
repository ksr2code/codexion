/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksmailov <ksmailov@student.42heilbronn.de  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 10:00:52 by ksmailov          #+#    #+#             */
/*   Updated: 2026/02/17 21:34:11 by ksmailov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int ac, char **av)
{
	t_config	cfg;
	t_sim		*sim;

	if (!parse_args(ac, av, &cfg))
		return (1);
	sim = init_simulation(&cfg);
	if (!sim)
		return (1);
	if (!create_coders(sim))
	{
		destroy_simulation(sim);
		return (1);
	}
	pthread_create(&sim->monitor_thread, NULL, monitor_routine, sim);
	wait_coders(sim);
	sim->burnout_detected = 1;
	pthread_join(sim->monitor_thread, NULL);
	destroy_simulation(sim);
	return (0);
}
