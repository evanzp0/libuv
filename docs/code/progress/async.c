#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <node/uv.h>
 
uv_async_t async;
uv_loop_t* loop;
 
void close_cb(uv_handle_t* handle);
void async_cb(uv_async_t* handle);
void sub_thread(void* arg);
 
void close_cb(uv_handle_t* handle)
{
    printf("close the async handle!\n");
}
 
void async_cb(uv_async_t* handle)
{
    printf("async_cb called!\n");
    uv_thread_t id = uv_thread_self();
    printf("thread id:%lu.\n", (long) id);
    uv_close((uv_handle_t*)&async, close_cb);    //如果async没有关闭，消息队列是会阻塞的
}
 
/**
 *
 */
void sub_thread(void* arg)
{
    uv_thread_t id = uv_thread_self();
    printf("sub thread id:%lu.\n", (long)id);
    uv_async_send(&async);
}
 
int main()
{
    loop = uv_default_loop();
 
    uv_thread_t id = uv_thread_self();
    printf("thread id:%lu.\n", (long)id);
 
    uv_async_init(loop, &async, async_cb);
 
    // 创建子线程
    uv_thread_t thread;
    uv_thread_create(&thread, sub_thread, NULL);
 
    uv_run(loop, UV_RUN_DEFAULT);
    uv_thread_join(&thread);    //等待子线程完成
 
    return 0;
}
