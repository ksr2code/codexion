/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksmailov <ksmailov@student.42heilbronn.de  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 10:00:52 by ksmailov          #+#    #+#             */
/*   Updated: 2026/02/17 19:48:19 by ksmailov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int argc, char **argv)
{
	t_config	cfg;
	t_sim		*sim;

	if (!parse_args(argc, argv, &cfg))
		return (1);
	sim = init_simulation(&cfg);
	if (!sim)
		return (1);
	create_coders(sim);
	wait_coders(sim);
	destroy_simulation(sim);
	return (0);
}
