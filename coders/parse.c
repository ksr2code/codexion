/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksmailov <ksmailov@student.42heilbronn.de  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 11:07:01 by ksmailov          #+#    #+#             */
/*   Updated: 2026/02/17 11:37:28 by ksmailov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	ft_isdigit(int c)
{
	return (c >= '0' && c <= '9');
}

static int	validate_args(int ac, char **av)
{
	int		i;
	int		j;
	long	num;

	if (ac != 9)
		return (0);
	i = 0;
	while (++i < ac - 1)
	{
		j = -1;
		num = 0;
		while (av[i][++j])
		{
			if (!ft_isdigit(av[i][j]))
				return (0);
			num = num * 10 + (av[i][j] - '0');
			if (num > INT_MAX)
				return (0);
		}
		if (num <= 0)
			return (0);
	}
	if (strcmp(av[8], "fifo") && strcmp(av[8], "edf"))
		return (0);
	return (1);
}

int	parse_args(int ac, char **av, t_config *cfg)
{
	if (!validate_args(ac, av))
	{
		fprintf(stderr, "Wrong arguments!\n");
		return (0);
	}
	cfg->number_of_coders = atoi(av[1]);
	cfg->time_to_burnout = atoi(av[2]);
	cfg->time_to_compile = atoi(av[3]);
	cfg->time_to_debug = atoi(av[4]);
	cfg->time_to_refactor = atoi(av[5]);
	cfg->number_of_compiles_required = atoi(av[6]);
	cfg->dongle_cooldown = atoi(av[7]);
	if (strcmp(av[8], "fifo") == 0)
		cfg->scheduler = FIFO;
	else
		cfg->scheduler = EDF;
	return (1);
}
