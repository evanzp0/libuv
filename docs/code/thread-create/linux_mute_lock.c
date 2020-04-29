/**************************************************************************************/
/*简介：互斥锁同步线程演示程序								 */
/*************************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

void *thread_function(void *arg);
pthread_mutex_t work_mutex; 

#define WORK_SIZE 1024

char work_area[WORK_SIZE];/*工作区*/
int time_to_exit = 0;/*退出程序标志*/

int main() 
{
    int res;
    pthread_t a_thread;
    void *thread_result;
    /*对互斥锁进行初始化*/
    res = pthread_mutex_init(&work_mutex, NULL);
    if (res != 0) 
    {
        perror("Mutex initialization failed");
        return 1;
    }
    res = pthread_create(&a_thread, NULL, thread_function, NULL);
    if (res != 0) 
    {
        perror("Thread creation failed");
        return 1;
    }

    /*给工作区加上锁，把文本读到它里面，
    然后给它解锁使它允许被其他线程访问*/
    pthread_mutex_lock(&work_mutex);
    printf("Input some text. Enter 'end' to finish\n");
    while(!time_to_exit) {
        fgets(work_area, WORK_SIZE, stdin);
        pthread_mutex_unlock(&work_mutex);
        while(1) {
            pthread_mutex_lock(&work_mutex);
            if (work_area[0] != '\0') 
	        {
                pthread_mutex_unlock(&work_mutex);
                sleep(1);
            } else {
                break;
            }
        }
    }
    pthread_mutex_unlock(&work_mutex);
    printf("\nWaiting for thread to finish...\n");
    res = pthread_join(a_thread, &thread_result);
    if (res != 0) 
    {
        perror("Thread join failed");
        return 1;
    }
    printf("Thread joined\n");
    pthread_mutex_destroy(&work_mutex);
    return 0;
}

void *thread_function(void *arg) 
{
    sleep(1);
    /*新线程首先试图对互斥量进行加锁*/
    pthread_mutex_lock(&work_mutex);

    while(strncmp("end", work_area, 3) != 0) 
    {
        printf("You input %zu characters\n", strlen(work_area) -1);
        /*把第一个字符设置为空字符已经完成了字符统计工作*/
        work_area[0] = '\0';
        /*对互斥量进行解锁并等待主线程的运行*/
        pthread_mutex_unlock(&work_mutex);
        sleep(1);
        /*周期性地尝试给互斥量加锁，如果加锁成功，
        就检查主线程是否有新的字符需要统计。
        如果还没有，就解开互斥量继续等待*/
        pthread_mutex_lock(&work_mutex);
        while (work_area[0] == '\0' ) 
	{
            pthread_mutex_unlock(&work_mutex);
            sleep(1);
            pthread_mutex_lock(&work_mutex);
        }
    }
    /*设置退出程序的标志*/
    time_to_exit = 1;
    work_area[0] = '\0';
    pthread_mutex_unlock(&work_mutex);
    pthread_exit(0);
}