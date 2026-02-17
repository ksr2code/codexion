/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksmailov <ksmailov@student.42heilbronn.de  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 15:02:23 by ksmailov          #+#    #+#             */
/*   Updated: 2026/02/17 15:49:36 by ksmailov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	create_coders(t_sim *sim)
{
	int	i;

	i = -1;
	while (++i < sim->num_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL, coder_routine,
				&sim->coders[i]) != 0)
			return (0);
	}
	return (1);
}

void	wait_coders(t_sim *sim)
{
	int	i;

	i = -1;
	while (++i < sim->num_coders)
		pthread_join(sim->coders[i].thread, NULL);
}

static int	dongle_available(t_dongle *dongle, t_config *cfg)
{
	return (get_timestamp_ms() >= dongle->cooldown_until);
}

static void	acquire_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	if (coder->left_dongle->id < coder->right_dongle->id)
	{
		first = coder->left_dongle;
		second = coder->right_dongle;
	}
	else
	{
		first = coder->right_dongle;
		second = coder->left_dongle;
	}
	while (!dongle_available(first, coder->cfg))
		msleep(1);
	pthread_mutex_lock(&first->mutex);
	log_state(coder->sim, coder->id, "has taken a dongle");
	while (!dongle_available(second, coder->cfg))
		msleep(1);
	pthread_mutex_lock(&second->mutex);
	log_state(coder->sim, coder->id, "has taken a dongle");
}

static void	release_dongles(t_coder *coder)
{
	long	now;

	now = get_timestamp_ms();
	coder->left_dongle->cooldown_until = now + coder->cfg->dongle_cooldown;
	coder->right_dongle->cooldown_until = now + coder->cfg->dongle_cooldown;
	pthread_mutex_unlock(&coder->left_dongle->mutex);
	pthread_mutex_unlock(&coder->right_dongle->mutex);
}

void	*coder_routine(void *data)
{
	t_coder	*coder;

	coder = data;
	while (coder->compiles_done < coder->cfg->number_of_compiles_required
		&& coder->alive)
	{
		acquire_dongles(coder);
		coder->last_compile_start = get_timestamp_ms();
		coder->compiles_done++;
		log_state(coder->sim, coder->id, "is compiling");
		msleep(coder->cfg->time_to_compile);
		log_state(coder->sim, coder->id, "is debugging");
		msleep(coder->cfg->time_to_debug);
		log_state(coder->sim, coder->id, "is refactoring");
		msleep(coder->cfg->time_to_refactor);
		release_dongles(coder);
	}
	return (NULL);
}
