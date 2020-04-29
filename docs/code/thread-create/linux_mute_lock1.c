#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

static pthread_mutex_t lock;
void *async_worker(void *arg) {
    int is_locked = 0;

    int retry = 0;
    while (1) {
        printf("is_locked = %d\n", is_locked);
        is_locked = pthread_mutex_trylock(&lock);

        if (is_locked == 0) {
            printf("get lock, thread id:%lu\n", (long)pthread_self());
            sleep(5);
            printf("release lock, thread id:%lu\n", (long)pthread_self());
            pthread_mutex_unlock(&lock);
            break;
        } else {
            printf("mutex is locked, thread id:%lu\n", (long)pthread_self());
            sleep(1);
            retry++;
            printf("re-get locked, thread id:%lu, retry %d\n", (long)pthread_self(), retry);
        }
    }
    pthread_exit(0);
}

int main() 
{
    int res = 0;
    void *thread_result;
    pthread_mutex_init(&lock, NULL);// init lock
    pthread_t nthread1,nthread2;

    res = pthread_create(&nthread1, NULL, async_worker, NULL);
    if (res != 0) {
        perror("Thread1 creation failed");
        return 1;
    }

    res = pthread_create(&nthread2, NULL, async_worker, NULL);
    if (res != 0) {
        perror("Thread2 creation failed");
        return 1;
    }

    pthread_join(nthread1, &thread_result);
    pthread_join(nthread2, &thread_result);

    pthread_mutex_destroy(&lock); // pthread_mutex_destroy 在free 前来 destory lock
    // free (&lock)
    return 0;
}
