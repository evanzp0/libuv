#include <stdio.h>
#include <unistd.h>
#include <uv.h>

uv_rwlock_t lock;
int num = 0;

void writer(void *arg) {
    uv_rwlock_wrlock(&lock);
    sleep(2);
    num++;
    printf("writer,num is %d\n",num);
    uv_rwlock_wrunlock(&lock);
}

void reader(void *arg) {
    uv_rwlock_rdlock(&lock);
    sleep(2);
    printf("reader,num is %d\n",num);
    uv_rwlock_rdunlock(&lock);
}

int main(int argc, char **argv) {
    uv_rwlock_init(&lock);

    uv_thread_t nthread1,nthread2,nthread3,nthread4;

    uv_thread_create(&nthread1, writer, NULL);
    uv_thread_create(&nthread2, writer, NULL);
    uv_thread_create(&nthread3, reader, NULL);
    uv_thread_create(&nthread4, reader, NULL);

    uv_thread_join(&nthread1);
    uv_thread_join(&nthread2);
    uv_thread_join(&nthread3);
    uv_thread_join(&nthread4);

    uv_rwlock_destroy(&lock);
}