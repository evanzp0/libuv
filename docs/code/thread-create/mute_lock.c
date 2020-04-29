#include <stdio.h>
#include <unistd.h>

#include <uv.h>

static uv_mutex_t lock;
void async_worker(void *arg) {
    int is_locked = 0;
    int retry = 0;
    while (1) {
        is_locked = uv_mutex_trylock(&lock);

        if (is_locked == 0) {
            printf("get lock, thread id:%lu\n",(long)uv_thread_self());
            sleep(5);
            printf("release lock, thread id:%lu\n",(long)uv_thread_self());
            uv_mutex_unlock(&lock);
            break;
        } else {
            printf("mutex is locked, thread id:%lu\n",(long)uv_thread_self());
            sleep(1);
            retry++;
            printf("re-get locked, thread id:%lu, retry %d\n",(long)uv_thread_self(), retry);
        }
    }
}

void sync_worker(void *arg) {
   
    uv_mutex_lock(&lock);
    sleep(2);
    printf("thread id:%lu\n",(long)uv_thread_self());
    uv_mutex_unlock(&lock);
}

int main(int argc, char **argv) {
    uv_mutex_init(&lock);
    uv_thread_t nthread1,nthread2;

    uv_thread_create(&nthread1, async_worker, NULL);
    uv_thread_create(&nthread2, async_worker, NULL);

    uv_thread_join(&nthread1);
    uv_thread_join(&nthread2);

    uv_mutex_destroy(&lock);
}

