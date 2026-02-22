/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksmailov <ksmailov@student.42heilbronn.de  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 13:32:39 by ksmailov          #+#    #+#             */
/*   Updated: 2026/02/22 13:40:30 by ksmailov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	swap_requests(t_request *r1, t_request *r2)
{
	t_request	tmp;

	tmp = *r1;
	*r1 = *r2;
	*r2 = tmp;
}

static void	sift_up(t_queue *q, int i)
{
	int	parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (q->requests[i].deadline >= q->requests[parent].deadline)
			break ;
		swap_requests(&q->requests[i], &q->requests[parent]);
		i = parent;
	}
}

static void	sift_down(t_queue *q, int i)
{
	int	left;
	int	right;
	int	smallest;

	while (1)
	{
		left = (2 * i) + 1;
		right = (2 * i) + 2;
		smallest = i;
		if (left < q->size)
		{
			if (q->requests[left].deadline < q->requests[smallest].deadline)
				smallest = left;
		}
		if (right < q->size)
		{
			if (q->requests[right].deadline < q->requests[smallest].deadline)
				smallest = right;
		}
		if (smallest == i)
			break ;
		swap_requests(&q->requests[i], &q->requests[smallest]);
		i = smallest;
	}
}

void	heap_insert(t_dongle *dongle, t_coder *coder)
{
	t_request	req;
	int			i;

	req.coder = coder;
	req.arrival_time = get_timestamp_ms();
	if (dongle->scheduler == EDF)
		req.deadline = coder->last_compile_start + coder->cfg->time_to_burnout;
	else
		req.deadline = req.arrival_time;
	i = dongle->queue.size;
	dongle->queue.requests[i] = req;
	dongle->queue.size++;
	sift_up(&dongle->queue, i);
}

t_coder	*heap_remove(t_dongle *dongle)
{
	t_coder	*winner;

	if (dongle->queue.size == 0)
	{
		return (NULL);
	}
	winner = dongle->queue.requests[0].coder;
	dongle->queue.size--;
	if (dongle->queue.size > 0)
	{
		dongle->queue.requests[0] = dongle->queue.requests[dongle->queue.size];
		sift_down(&dongle->queue, 0);
	}
	return (winner);
}
