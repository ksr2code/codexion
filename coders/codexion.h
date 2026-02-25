/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ksmailov <ksmailov@student.42heilbronn.de  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/17 10:01:05 by ksmailov          #+#    #+#             */
/*   Updated: 2026/02/22 16:18:02 by ksmailov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <limits.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>
# include <pthread.h>

typedef struct s_sim	t_sim;
typedef struct s_coder	t_coder;

typedef enum e_scheduler
{
	FIFO,
	EDF
}	t_scheduler;

typedef struct s_config
{
	int				number_of_coders;
	int				time_to_burnout;
	int				time_to_compile;
	int				time_to_debug;
	int				time_to_refactor;
	int				number_of_compiles_required;
	int				dongle_cooldown;
	t_scheduler		scheduler;
}	t_config;

typedef struct s_request
{
	t_coder		*coder;
	long		arrival_time;
	long		deadline;
}				t_request;

typedef struct s_queue
{
	t_request	*requests;
	int			size;
}	t_queue;

typedef struct s_dongle
{
	int					id;
	pthread_mutex_t		mutex;
	pthread_cond_t		cond;
	long				cooldown_until;
	int					available;
	t_queue				queue;
	t_scheduler			scheduler;
}	t_dongle;

typedef struct s_coder
{
	int				id;
	pthread_t		thread;
	t_dongle		*left_dongle;
	t_dongle		*right_dongle;
	long			last_compile_start;
	int				compiles_done;
	int				alive;
	t_config		*cfg;
	t_sim			*sim;
	pthread_mutex_t	compile_mutex;
}	t_coder;

typedef struct s_sim
{
	pthread_mutex_t		log_mutex;
	long				start_time;
	t_coder				*coders;
	t_dongle			*dongles;
	int					num_coders;
	int					burnout_detected;
	pthread_t			monitor_thread;
}	t_sim;

int		parse_args(int ac, char **av, t_config *cfg);
long	get_timestamp_ms(void);
void	msleep(t_sim *sim, long ms);
void	log_state(t_sim *sim, int coder_id, const char *action);
void	log_burnout(t_sim *sim, int coder_id);
t_sim	*init_simulation(t_config *cfg);
void	destroy_simulation(t_sim *sim);
int		init_resources(t_sim *sim, t_config *cfg);
void	*coder_routine(void *data);
int		create_coders(t_sim *sim);
void	wait_coders(t_sim *sim);
void	acquire_lower_first(t_coder *coder);
void	acquire_dongle(t_coder *coder, t_dongle *dongle);
void	release_dongles(t_coder *coder);
void	get_timeout_ts(struct timespec *ts, long timeout_ms);
void	*monitor_routine(void *data);
int		create_monitor(t_sim *sim);
void	wait_monitor(t_sim *sim);
void	heap_insert(t_dongle *dongle, t_coder *coder);
t_coder	*heap_remove(t_dongle *dongle);

#endif
