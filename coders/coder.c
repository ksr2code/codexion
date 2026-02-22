/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksmailov <ksmailov@student.42heilbronn.de  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 15:02:23 by ksmailov          #+#    #+#             */
/*   Updated: 2026/02/17 22:11:12 by ksmailov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	should_stop(t_coder *coder)
{
	return (coder->sim->burnout_detected
		|| coder->compiles_done >= coder->cfg->number_of_compiles_required);
}

static void	do_compile_phase(t_coder *coder)
{
	pthread_mutex_lock(&coder->compile_mutex);
	coder->last_compile_start = get_timestamp_ms();
	pthread_mutex_unlock(&coder->compile_mutex);
	coder->compiles_done++;
	log_state(coder->sim, coder->id, "is compiling");
	msleep(coder->sim, coder->cfg->time_to_compile);
}

static void	do_debug_phase(t_coder *coder)
{
	log_state(coder->sim, coder->id, "is debugging");
	msleep(coder->sim, coder->cfg->time_to_debug);
}

static void	do_refactor_phase(t_coder *coder)
{
	log_state(coder->sim, coder->id, "is refactoring");
	msleep(coder->sim, coder->cfg->time_to_refactor);
}

void	*coder_routine(void *data)
{
	t_coder	*coder;

	coder = (t_coder *)data;
	while (!should_stop(coder))
	{
		acquire_lower_first(coder);
		if (coder->sim->burnout_detected)
			break ;
		do_compile_phase(coder);
		if (coder->sim->burnout_detected)
			break ;
		do_debug_phase(coder);
		if (coder->sim->burnout_detected)
			break ;
		do_refactor_phase(coder);
		if (coder->sim->burnout_detected)
			break ;
		release_dongles(coder);
	}
	coder->alive = 0;
	return (NULL);
}
