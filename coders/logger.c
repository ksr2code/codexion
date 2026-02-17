/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logger.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksmailov <ksmailov@student.42heilbronn.de  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 13:30:55 by ksmailov          #+#    #+#             */
/*   Updated: 2026/02/17 14:05:55 by ksmailov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	log_state(t_sim *sim, int coder_id, const char *action)
{
	long	timestamp;

	timestamp = get_timestamp_ms() - sim->start_time;
	pthread_mutex_lock(&sim->log_mutex);
	printf("%ld %d %s\n", timestamp, coder_id, action);
	fflush(stdout);
	pthread_mutex_unlock(&sim->log_mutex);
}

void	log_burnout(t_sim *sim, int coder_id)
{
	long	timestamp;

	timestamp = get_timestamp_ms() - sim->start_time;
	pthread_mutex_lock(&sim->log_mutex);
	printf("%ld %d burned out\n", timestamp, coder_id);
	fflush(stdout);
	pthread_mutex_unlock(&sim->log_mutex);
}
