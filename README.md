*This project has been created as part of the 42 curriculum by ksmailov*

# Codexion

## Description

Codexion is a concurrency challenge that simulates multiple coders working in a shared
co-working space where USB dongles are limited resources. Each coder must compile quantum
code by simultaneously acquiring two dongles (left and right), then debug and refactor
in cycles. The simulation continues until either a coder burns out (fails to compile
within the deadline) or all coders complete the required number of compiles.

The project demonstrates advanced concurrent programming techniques including:
- Multi-threaded synchronization using POSIX threads (pthread)
- Priority-based scheduling algorithms (FIFO and EDF)
- Resource allocation and deadlock prevention
- Cooldown mechanisms for fair resource sharing
- Precise burnout detection and monitoring

## Instructions

### Compilation

```bash
make
```

The Makefile compiles all source files with the required flags:
- `-Wall -Wextra -Werror` for strict compiler warnings
- `-pthread` for POSIX thread support

### Execution

```bash
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> <time_to_debug> \
           <time_to_refactor> <number_of_compiles_required> <dongle_cooldown> <scheduler>
```

**Arguments (all mandatory):**
- `number_of_coders`: Number of coder threads and dongles (must be positive integer)
- `time_to_burnout`: Maximum milliseconds since last compile before burnout (must be positive integer)
- `time_to_compile`: Milliseconds to compile while holding both dongles (must be positive integer)
- `time_to_debug`: Milliseconds spent debugging after compile (must be positive integer)
- `time_to_refactor`: Milliseconds spent refactoring after debug (must be positive integer)
- `number_of_compiles_required`: Compiles each coder must complete to finish successfully (must be non-negative integer)
- `dongle_cooldown`: Milliseconds after release before dongle can be acquired again (must be non-negative integer)
- `scheduler`: Arbitration policy — must be either `fifo` or `edf`
  - `fifo`: First-In-First-Out based on arrival time
  - `edf`: Earliest Deadline First (burnout deadline = last_compile_start + time_to_burnout)

**Invalid arguments** will print "Wrong arguments!" and exit with code 1.

### Example

```bash
./codexion 3 200 100 50 30 2 10 fifo
```

This creates 3 coders who must compile every 200ms, with 100ms compile time, 50ms debug,
30ms refactor, needing 2 compiles each, 10ms cooldown, using FIFO scheduling.

### Output Format

The program outputs state changes with millisecond timestamps:
```
<timestamp> <coder_id> has taken a dongle
<timestamp> <coder_id> has taken a dongle
<timestamp> <coder_id> is compiling
<timestamp> <coder_id> is debugging
<timestamp> <coder_id> is refactoring
<timestamp> <coder_id> burned out
```

Each "is compiling" is preceded by exactly two "has taken a dongle" messages —
one for each dongle acquired. Both appear back-to-back after both dongles are secured.

### Cleanup

```bash
make clean
make fclean
```

## Blocking Cases Handled

### Deadlock Prevention

Deadlock requires all four Coffman conditions to hold simultaneously. This implementation
breaks the **Circular Wait** condition:

**Coffman's Conditions Analysis:**
1. **Mutual Exclusion** — Required and unavoidable (each dongle used by one coder at a time)
2. **Hold and Wait** — Present (coder briefly holds first dongle's mutex while acquiring second)
3. **No Preemption** — Coders voluntarily release dongles; no preemption needed
4. **Circular Wait** — **BROKEN** by consistent lower-ID-first resource ordering

**Implementation Details:**
- `get_ordered()` always assigns the lower-ID dongle as `first`, higher-ID as `second`
- All coders acquire `first` before `second` — no exceptions
- This creates a strict global ordering that makes circular dependency impossible

**Example — Why Circular Wait Cannot Occur:**
```
WITHOUT ordering (dangerous):
  Coder 4 holds dongle[3], waiting for dongle[0]  <- forms a circle!
  Coder 1 holds dongle[0], waiting for dongle[1]
  -> DEADLOCK

WITH lower-ID-first ordering:
  Coder 4 must acquire dongle[0] first (lower ID)
  Coder 4 cannot hold dongle[3] while waiting for dongle[0]
  -> No circular dependency possible
```

### Starvation Prevention

**EDF Scheduler:**
- Coders closest to burnout deadline receive highest priority
- Even if another coder arrived earlier, those at risk of burning out are served first
- Guarantees liveness provided simulation parameters are feasible

**FIFO Scheduler:**
- First request to arrive is first served
- Fairness based on arrival order
- No coder can be permanently blocked while the system is progressing

### Cooldown Handling

After releasing dongles, they enter a cooldown period before becoming available again:

**Implementation:**
```c
now = get_timestamp_ms();
dongle->available = 1;
dongle->cooldown_until = now + cfg->dongle_cooldown;
```

**Cooldown Check in Acquisition:**
```c
dongle_available() returns true when:
    (dongle->available == 1) &&
    (current_time >= dongle->cooldown_until)
```

This ensures:
- Dongles cannot be acquired immediately after release
- Cooldown period is respected exactly
- Multiple coders do not race for just-released dongles

### Atomic Two-Dongle Acquisition

To avoid holding one dongle while waiting for the other, coders check **both dongles'
availability** before committing to acquiring either:

**Two-Phase Acquisition:**
1. **Phase 1:** Add request to both dongle queues simultaneously (non-blocking)
2. **Phase 2:** Wait under `pair_mutex` until both dongles are available and coder
   is at the front of both queues, then acquire both atomically

**Benefits:**
- Never holds one dongle while waiting for the other
- Other coders can use either dongle in the meantime
- Logs "has taken a dongle" twice only after both are fully secured
- Matches expected output format: both log lines always appear back-to-back

### Precise Burnout Detection

A dedicated monitor thread continuously checks each coder's time since last compile:

**Detection Logic:**
```c
elapsed = current_time - coder->last_compile_start;
if (elapsed >= time_to_burnout) {
    set_burnout_flag();       // under pair_mutex - stops all coders atomically
    broadcast_pair_cond();    // wakes all waiting coders immediately
    log_burnout();            // logged after flag set - no post-burnout logs possible
}
```

**Precision:**
- Monitor checks every 1ms
- Burnout log appears within ~1ms of actual burnout time
- `last_compile_start` protected by dedicated `compile_mutex` per coder

**Burnout Recovery:**
- All coder threads check `is_burnout()` at every phase transition
- Monitor broadcasts on `pair_cond` to wake all coders blocked in acquisition
- Coders exit gracefully without deadlocking or corrupting state
- Simulation terminates cleanly with full resource cleanup

### Log Serialization

All output is protected by a dedicated log mutex to prevent interleaved messages:

**Thread-Safe Logging:**
```c
pthread_mutex_lock(&sim->log_mutex);
printf("%ld %d %s\n", timestamp, coder_id, action);
fflush(stdout);
pthread_mutex_unlock(&sim->log_mutex);
```

**Prevents:**
```
CORRUPTED: "0 1 is com0 2 is debugging"
CORRECT:   "0 1 is compiling"
           "0 2 is debugging"
```

**Ensures:**
- Each state change appears on a complete, uninterrupted line
- Timestamps are accurate to the moment of logging
- Output is fully parseable for grading

## Thread Synchronization Mechanisms

### pthread_mutex_t (Mutex)

**Purpose:** Protect shared data from concurrent access

**Four mutexes used:**

1. **`dongle->mutex`** — one per dongle
   - Protects: `available`, `cooldown_until`, heap queue operations
   - Ensures only one thread modifies dongle state at a time

2. **`coder->compile_mutex`** — one per coder
   - Protects: `coder->last_compile_start`
   - Allows monitor thread to safely read timestamp while coder writes it
   - Kept separate from `pair_mutex` to avoid adding contention on the hot path

3. **`sim->log_mutex`** — global
   - Protects: all `printf` calls
   - Ensures complete, non-interleaved log lines

4. **`sim->pair_mutex`** — global
   - Protects: `burnout_detected` flag (all reads and writes)
   - Guards the atomic two-dongle availability check
   - Coordinates coder threads during acquisition and burnout
   - Note: `dongle_available()` reads dongle fields without `dongle->mutex`
     because `pair_mutex` is always held at that point, and all writers of
     those fields also hold `pair_mutex` — making the read safe

**Race Condition Prevention Example:**
```c
// Without mutex - race condition:
if (dongle->available)    // Thread A reads: true
    take_dongle();        // Thread B reads: true simultaneously -> both take it!

// With pair_mutex - safe:
pthread_mutex_lock(&pair_mutex);
if (dongle_available(first) && dongle_available(second))
    get_dongles(first, second);   // only one thread can be here at a time
pthread_mutex_unlock(&pair_mutex);
```

### pthread_cond_t (Condition Variable)

**Purpose:** Efficient waiting for state changes without busy-waiting

**One condition variable used:**

**`sim->pair_cond`** — global, paired with `sim->pair_mutex`
- Coders wait here when not at front of both queues or dongles unavailable
- Broadcast on any dongle release wakes all waiting coders
- Broadcast on burnout detection wakes all coders immediately
- Single condition variable covers both use cases cleanly

**Coder waiting pattern:**
```c
pthread_mutex_lock(&pair_mutex);
while (!both_dongles_available() && !is_burnout()) {
    // Atomically releases pair_mutex and blocks
    pthread_cond_timedwait(&pair_cond, &pair_mutex, &timeout);
    // On wake: atomically re-acquires pair_mutex, re-checks conditions
}
pthread_mutex_unlock(&pair_mutex);
```

**Release and signal pattern:**
```c
// Update dongle state under dongle->mutex
pthread_mutex_lock(&dongle->mutex);
dongle->available = 1;
pthread_mutex_unlock(&dongle->mutex);

// Signal all waiters under pair_mutex
pthread_mutex_lock(&pair_mutex);
pthread_cond_broadcast(&pair_cond);
pthread_mutex_unlock(&pair_mutex);
```

**Why `timedwait` with 10ms timeout:**
- Prevents indefinite blocking if a signal is missed between checks
- Allows periodic re-check of burnout flag as a safety net
- 10ms is small enough to not affect burnout detection accuracy

### Custom Pattern: Pair Lock

**Problem with per-dongle condition variables:**
```c
// Traditional (broken for two-resource acquisition):
pthread_mutex_lock(&dongle_0->mutex);
pthread_cond_wait(&dongle_0->cond, &dongle_0->mutex);  // holds dongle_0 while waiting!
pthread_mutex_lock(&dongle_1->mutex);                  // neighbor blocked on dongle_0
```

**Pair Lock solution:**
```c
// Announce intent in both queues (outside pair_mutex)
add_to_queue(dongle_0, dongle_1);

// Wait until BOTH are ready under one shared mutex
pthread_mutex_lock(&pair_mutex);
while (!both_available()) {
    pthread_cond_timedwait(&pair_cond, &pair_mutex, &ts);
}
// Take both atomically - no gap, no race
get_dongles(dongle_0, dongle_1);
pthread_mutex_unlock(&pair_mutex);
```

**Advantages over per-dongle approach:**
- Never holds one dongle while waiting for the other
- Single condition variable — no missed signals from the "wrong" cond
- Fewer total mutex operations
- Correct `pthread_cond_timedwait` usage (exactly one mutex held)

### Priority Queue (Min-Heap)

**Purpose:** Implement FIFO and EDF scheduling per dongle

**Data structure:** Min-heap — smallest deadline always at index 0

**FIFO mode:**
```c
req.deadline = req.arrival_time;  // earlier arrival = smaller deadline = higher priority
```

**EDF mode:**
```c
req.deadline = coder->last_compile_start + coder->cfg->time_to_burnout;
// coder closest to burnout has smallest deadline = highest priority
```

**Properties:**
- Complete binary tree in array form: parent at `i`, children at `2i+1` and `2i+2`
- Insert and remove both O(log n) — effectively O(1) since max queue size is 2
  (each dongle is shared by exactly 2 adjacent coders in the circular topology)
- Operations protected by `dongle->mutex`

## Resources

### Documentation and References

**POSIX Threads:**
- [pthread_mutex_init(3)](https://man7.org/linux/man-pages/man3/pthread_mutex_init.3.html)
- [pthread_cond_timedwait(3)](https://man7.org/linux/man-pages/man3/pthread_cond_timedwait.3.html)
- [pthread_create(3)](https://man7.org/linux/man-pages/man3/pthread_create.3.html)

**Concurrency Concepts:**
- Coffman Conditions — deadlock analysis framework
- Resource Hierarchy Solution — deadlock prevention via ordered acquisition
- Dining Philosophers Problem — classical analogy for this project
- Producer-Consumer Pattern — monitor thread as consumer of coder state

**Data Structures:**
- Min-heap for priority queue implementation

**C Standards:**
- 42 Norm compliance — 25 lines per function, no global variables
- `-Wall -Wextra -Werror` strict compilation
- Memory management: malloc/free with full error checking

### AI Usage

This project was developed with assistance from AI tools for:

**Learning and Understanding:**
- Explaining concurrency concepts (mutexes, condition variables, deadlocks)
- Understanding the dining philosophers problem and classical solutions
- Clarifying POSIX thread API semantics and edge cases
- Analyzing race conditions and timing issues in concurrent code

**Code Review and Debugging:**
- Identifying potential race conditions and missed signal scenarios
- Suggesting the pair lock pattern for atomic two-dongle acquisition
- Verifying correct and symmetric use of synchronization primitives
- Catching issues like unprotected `burnout_detected` reads and post-burnout logs

**Documentation:**
- Structuring this README with clear explanations of complex concepts
- Describing synchronization mechanisms and blocking case solutions

**Implementation Decisions:**
- Choosing between per-dongle vs. pair lock synchronization approach
- Designing the two-phase atomic acquisition strategy
- Determining appropriate mutex granularity (4 mutexes, each with clear purpose)

**What AI Did NOT Do:**
- Generate production code without review and understanding
- Make design decisions without considering trade-offs
- Skip thorough testing and verification of edge cases

All code was reviewed, understood, and tested before integration.
AI served as a learning assistant and code reviewer, not a code generator.
