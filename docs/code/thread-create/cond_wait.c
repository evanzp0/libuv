#include <stdio.h>
#include <unistd.h>
#include <uv.h>

uv_mutex_t mutex;
uv_cond_t cond;
int num = 0;

void worker1(void *arg) {
    while(1){
        uv_mutex_lock(&mutex);
        sleep(2);
        num++;
        printf("worker1 waiting for num > 0\n");
        uv_cond_wait(&cond, &mutex); // 解锁 -> 阻塞 -> 收到信号 -> 激活 -> 加锁 
        printf("worker1 num is %d\n",num);
        num--;
        printf("worker1 num = %d \n", num);
        uv_mutex_unlock(&mutex);
    }
}

void worker2(void *arg) {
    while(1){
        uv_mutex_lock(&mutex);
        sleep(2);
        
        if(num>0){
            printf("worker2 num = %d \n", num);
            uv_cond_signal(&cond);
        }
        uv_mutex_unlock(&mutex);
        sleep(2);
    }
}

int main(int argc, char **argv) {
    uv_mutex_init(&mutex);
    uv_cond_init(&cond);
    uv_thread_t nthread1,nthread2;
    uv_thread_create(&nthread1, worker1, NULL);
    uv_thread_create(&nthread2, worker2, NULL);
    uv_thread_join(&nthread1);
    uv_thread_join(&nthread2);
}