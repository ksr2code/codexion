/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksmailov <ksmailov@student.42heilbronn.de  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 13:34:42 by ksmailov          #+#    #+#             */
/*   Updated: 2026/02/17 22:07:21 by ksmailov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <pthread.h>

long	get_timestamp_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void	msleep(t_sim *sim, long ms)
{
	long	start;
	long	elapsed;

	start = get_timestamp_ms();
	while (!burnout_detected(sim))
	{
		elapsed = get_timestamp_ms() - start;
		if (elapsed >= ms)
			break ;
		usleep(1000);
	}
}
