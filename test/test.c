#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;
int             ready = 0;

static struct timespec get_timeout(long ms)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec  += ms / 1000;
    ts.tv_nsec += (ms % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }
    return ts;
}

void *waiter(void *arg)
{
    int id = *(int *)arg;
    struct timespec ts;

    pthread_mutex_lock(&mutex);
    while (!ready)
    {
        ts = get_timeout(200);  // 200ms timeout
        int ret = pthread_cond_timedwait(&cond, &mutex, &ts);
        if (ret == 0)
            printf("thread %d: woken by signal\n", id);
        else
            printf("thread %d: timeout fired\n", id);
    }
    printf("thread %d: ready=1, exiting\n", id);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void *signaler(void *arg)
{
    (void)arg;
    usleep(500000);  // wait 500ms then signal
    printf("signaler: broadcasting\n");
    pthread_mutex_lock(&mutex);
    ready = 1;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(void)
{
    pthread_t w1, w2, w3, s;
    int ids[3] = {1, 2, 3};

    pthread_create(&w1, NULL, waiter, &ids[0]);
    pthread_create(&w2, NULL, waiter, &ids[1]);
    pthread_create(&w3, NULL, waiter, &ids[2]);
    pthread_create(&s,  NULL, signaler, NULL);

    pthread_join(w1, NULL);
    pthread_join(w2, NULL);
    pthread_join(w3, NULL);
    pthread_join(s,  NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}
