/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksmailov <ksmailov@student.42heilbronn.de  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 13:32:39 by ksmailov          #+#    #+#             */
/*   Updated: 2026/03/11 06:52:06 by ksmailov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	has_priority(t_request *a, t_request *b)
{
	return (a->deadline < b->deadline);
}

static void	shift_up(t_queue *q, int i)
{
	int	parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!has_priority(&q->requests[i], &q->requests[parent]))
			break ;
		swap_requests(&q->requests[i], &q->requests[parent]);
		i = parent;
	}
}

static void	shift_down(t_queue *q, int i)
{
	int	left;
	int	right;
	int	smallest;

	while (1)
	{
		left = (2 * i) + 1;
		right = (2 * i) + 2;
		smallest = i;
		if (left < q->size && has_priority(&q->requests[left],
				&q->requests[smallest]))
			smallest = left;
		if (right < q->size && has_priority(&q->requests[right],
				&q->requests[smallest]))
			smallest = right;
		if (smallest == i)
			break ;
		swap_requests(&q->requests[i], &q->requests[smallest]);
		i = smallest;
	}
}

void	heap_insert(t_queue *q, t_coder *coder, t_scheduler scheduler)
{
	t_request	req;
	int			i;

	req.coder = coder;
	req.arrival_time = get_timestamp_ms();
	if (scheduler == EDF)
		req.deadline = coder->last_compile_start + coder->cfg->time_to_burnout;
	else
		req.deadline = q->fifo_counter++;
	i = q->size;
	q->requests[i] = req;
	q->size++;
	shift_up(q, i);
}

t_coder	*heap_remove(t_queue *q)
{
	t_coder	*winner;

	if (q->size == 0)
		return (NULL);
	winner = q->requests[0].coder;
	q->size--;
	if (q->size > 0)
	{
		q->requests[0] = q->requests[q->size];
		shift_down(q, 0);
	}
	return (winner);
}
