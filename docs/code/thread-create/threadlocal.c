#include <stdio.h>
#include <unistd.h>
#include <uv.h>

uv_mutex_t lock;
uv_key_t key;
uv_barrier_t barrier;
int num = 0;

void after(){
    uv_barrier_wait(&barrier);
    printf("thread id is:%lu, my_num = %d\n", (long) uv_thread_self(), *((int*)uv_key_get(&key)));
}

void worker1(void *arg) {
    int num = 1; // 临时变量，但是在堆栈底部不会丢
    uv_key_set(&key,&num);
    after();
}

void worker2(void *arg) {
    int num = 2;
    uv_key_set(&key,&num);
    after();
}

void worker3(void *arg) {
    int num = 3;
    uv_key_set(&key,&num);
    after();
}

int main(int argc, char **argv) {
    uv_mutex_init(&lock);
    uv_key_create(&key);
    uv_barrier_init(&barrier,3);
    uv_thread_t nthread1,nthread2,nthread3;
    uv_thread_create(&nthread1, worker1, NULL);
    uv_thread_create(&nthread2, worker2, NULL);
    uv_thread_create(&nthread3, worker3, NULL);
    uv_thread_join(&nthread1);
    uv_thread_join(&nthread2);
    uv_thread_join(&nthread3);
    uv_key_delete(&key);
}