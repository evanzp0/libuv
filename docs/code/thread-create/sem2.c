#include <stdio.h>
#include <unistd.h>
#include <uv.h>

uv_sem_t sem;
int num = 0;
void reader(void *arg) {
    // 1、t1 执行时发现 S == 0，于是都被阻塞
    // 2、t2 执行时发现 S == 0，于是都被阻塞
    // 4.1 t1 被激活，发现 S == 1，于是将S 设为 0，并继续执行
    // 4.2 t2 被激活，发现 S == 0，于是都被阻塞
    // 6、t2被激活，发现 S == 1，于是将S 设为 0，并继续执行
    uv_sem_wait(&sem); 
    // 5、t1 将 S 设为 1，并继续执行
    uv_sem_post(&sem);
    sleep(2);
    printf("reader,num is%d\n",num);
}
void writer(void *arg) {
  printf("writer,num is%d\n",num);
  num++;
  // 3、t3执行时，将 S 设 1
  uv_sem_post(&sem);
}
int main(int argc, char **argv) {
  uv_sem_init(&sem,0);
  uv_thread_t nthread1,nthread2,nthread3;
  uv_thread_create(&nthread1, reader, NULL);
  uv_thread_create(&nthread2, reader, NULL);
  uv_thread_create(&nthread3, writer, NULL);
  uv_thread_join(&nthread1);
  uv_thread_join(&nthread2);
  uv_thread_join(&nthread3);
}