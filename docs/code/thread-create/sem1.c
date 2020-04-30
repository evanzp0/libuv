#include <stdio.h>
#include <unistd.h>
#include <uv.h>

uv_sem_t sem;
void worker(void *arg) {
    // 1.第一个线程看到 信号量 > 0，它将信号量设为 0,并继续向下执行；
    // 2.第二个线程看到信号量 == 0，于是被block。
    // 3.当第二个线程收到第一个线程调用 uv_sem_post 后，发来的信号量为1时，它将信号量设为0，并继续向下执行
    uv_sem_wait(&sem); 
    sleep(2);
    printf("thread id is:%lu\n",(long)uv_thread_self());
    uv_sem_post(&sem); // 将信号量设为1，其他被uv_sem_wait阻塞的线程会被激活。
}
int main(int argc, char **argv) {
  uv_sem_init(&sem,1); // 将信号量设为1
  uv_thread_t nthread1,nthread2;
  uv_thread_create(&nthread1, worker, NULL); //激活 nthread1
  uv_thread_create(&nthread2, worker, NULL); //激活 nthread2
  uv_thread_join(&nthread1); //等待 nthread1 完成
  uv_thread_join(&nthread2); //等待 nthread2 完成
  
  printf("over! \n");
}