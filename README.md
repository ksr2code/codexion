*This project has been created as part of the 42 curriculum by ksmailov*

# Codexion

## Description

Codexion simulates multiple coders competing for shared USB dongles in a circular
co-working hub. Each coder must acquire two adjacent dongles simultaneously to compile
quantum code, then debug and refactor in cycles. The simulation ends when a coder burns
out or all coders complete the required number of compiles.

Key techniques demonstrated:
- Multi-threaded synchronization using POSIX threads
- FIFO and EDF scheduling via a min-heap priority queue
- Deadlock-free atomic two-dongle acquisition using a shared pair lock
- Condition variable-based cooldown wakeup via a monitor thread
- Precise burnout detection within ~1ms

## Instructions

### Compilation

```bash
make
```

Compiles with `-Wall -Wextra -Werror -pthread`.

### Execution

```bash
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> <time_to_debug> \
           <time_to_refactor> <number_of_compiles_required> <dongle_cooldown> <scheduler>
```

**Arguments (all mandatory):**
- `number_of_coders`: Number of coder threads and dongles
- `time_to_burnout`: Max ms since last compile start before burnout
- `time_to_compile`: Ms to compile while holding both dongles
- `time_to_debug`: Ms spent debugging
- `time_to_refactor`: Ms spent refactoring
- `number_of_compiles_required`: Compiles each coder must complete
- `dongle_cooldown`: Ms after release before dongle can be reacquired
- `scheduler`: `fifo` or `edf`

Invalid arguments print `"Wrong arguments!"` and exit with code 1.

### Example

```bash
./codexion 4 333 100 100 100 3 10 edf
```

### Output Format

```
<timestamp_ms> <coder_id> has taken a dongle
<timestamp_ms> <coder_id> has taken a dongle
<timestamp_ms> <coder_id> is compiling
<timestamp_ms> <coder_id> is debugging
<timestamp_ms> <coder_id> is refactoring
<timestamp_ms> <coder_id> burned out
```

### Cleanup

```bash
make fclean
```

## Blocking Cases Handled

### Deadlock Prevention

Both Coffman conditions **Hold-and-Wait** and **Circular Wait** are eliminated by
acquiring both dongles atomically under a single `pair_mutex`. A coder never holds
one dongle while waiting for the other:

```c
pthread_mutex_lock(&sim->pair_mutex);
heap_insert(&sim->queue, coder, sim->scheduler);
while (!sim->burnout)
{
    if (coder_can_compile(coder))
    {
        coder->left_dongle->available = 0;
        coder->right_dongle->available = 0;
        heap_remove(&sim->queue);
        break;
    }
    pthread_cond_wait(&sim->pair_cond, &sim->pair_mutex);
}
pthread_mutex_unlock(&sim->pair_mutex);
```

### Starvation Prevention

All coders register in a shared global min-heap before waiting. Only the heap root
(highest-priority coder) may compile at any time:

```c
if (sim->queue.size == 0 || sim->queue.requests[0].coder != coder)
    return (0);
```

- **EDF**: deadline = `last_compile_start + time_to_burnout` — coder closest to burnout compiles first
- **FIFO**: deadline = monotonically increasing arrival counter — strict first-come-first-served

### Cooldown Handling

After releasing dongles, a cooldown prevents immediate reacquisition:

```c
coder->left_dongle->cooldown_until = get_timestamp_ms() + cfg->dongle_cooldown;
```

Since coders use `pthread_cond_wait`, the monitor broadcasts on `pair_cond` every 1ms,
waking blocked coders to recheck cooldown expiry without busy-waiting.

### Burnout Detection

A dedicated monitor thread checks each coder's elapsed time since last compile every 1ms:

```c
elapsed = get_timestamp_ms() - coder->last_compile_start;
if (elapsed >= coder->cfg->time_to_burnout)
{
    pthread_mutex_lock(&sim->pair_mutex);
    sim->burnout = 1;
    pthread_cond_broadcast(&sim->pair_cond);
    pthread_mutex_unlock(&sim->pair_mutex);
    log_burnout(sim, coder->id);
}
```

`last_compile_start` is protected by a per-coder `compile_mutex` to prevent data
races between the coder writing it and the monitor reading it.

### Log Serialization

All output is protected by `log_mutex` to prevent interleaved messages:

```c
pthread_mutex_lock(&sim->log_mutex);
printf("%ld %d %s\n", timestamp, coder_id, action);
pthread_mutex_unlock(&sim->log_mutex);
```

## Thread Synchronization Mechanisms

### Mutexes

Three mutexes are used:

**`sim->pair_mutex`** — global pair lock
Protects `sim->burnout`, `sim->queue`, and all dongle `available`/`cooldown_until`
fields. All dongle acquisition, release, and burnout state changes go through this
single mutex, preventing deadlock across all threads.

**`coder->compile_mutex`** — one per coder
Protects `coder->last_compile_start` and `coder->alive`. Kept separate from
`pair_mutex` to avoid contention between the monitor and the coder hot path.

**`sim->log_mutex`** — global log lock
Protects all `printf` calls to guarantee non-interleaved output.

### Condition Variable

**`sim->pair_cond`** — paired with `sim->pair_mutex`

Coders block here when not the queue root or when dongles are unavailable. All
wakeup sources broadcast explicitly:

```
1. release_dongles()      — dongles available again after use
2. monitor_routine()      — periodic 1ms tick for cooldown expiry
3. check_coder_burnout()  — burnout detected, simulation ending
4. create_monitor()       — monitor thread creation failure
5. wait_monitor()         — simulation ending normally
```

Using `pthread_cond_wait` instead of `pthread_cond_timedwait` keeps the code clean.
The monitor's 1ms periodic broadcast acts as the timeout mechanism for cooldown
expiry, avoiding TSan false positives that arise with `timedwait`.

### Priority Queue (Min-Heap)

A binary min-heap stores coder requests with the highest-priority coder at index 0.

- **FIFO**: `deadline = fifo_counter++`
- **EDF**: `deadline = last_compile_start + time_to_burnout`

Insert and remove are O(log N). The root-only compilation check ensures fair,
starvation-free scheduling for all coders.

## Resources

- [pthread_mutex_init(3)](https://man7.org/linux/man-pages/man3/pthread_mutex_init.3.html)
- [pthread_cond_wait(3)](https://man7.org/linux/man-pages/man3/pthread_cond_wait.3p.html)
- [pthread_cond_broadcast(3)](https://man7.org/linux/man-pages/man3/pthread_cond_broadcast.3p.html)
- [gettimeofday(2)](https://man7.org/linux/man-pages/man2/gettimeofday.2.html)
- [Valgrind Helgrind Manual](https://valgrind.org/docs/manual/hg-manual.html)
- Dining Philosophers Problem — classical analogy for this project
- Coffman Conditions — deadlock analysis framework
- Earliest Deadline First scheduling — real-time systems theory

### AI Usage

AI was used as a learning assistant and code reviewer throughout this project:

- Understanding condition variable semantics and spurious wakeup handling
- Analyzing Coffman conditions in the context of two-dongle acquisition
- Deciding between `cond_wait` + periodic monitor broadcast vs. `cond_timedwait`
- Identifying missing broadcasts on burnout paths and dangling heap entries
- Verifying `last_compile_start` protection under `compile_mutex`
- Structuring this README to meet subject requirements

All suggestions were reviewed, understood, and tested before integration.
No code was used without full comprehension of its behavior and trade-offs.
