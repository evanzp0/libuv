#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uv.h>

uv_loop_t* create_loop()
{
    uv_loop_t *loop = malloc(sizeof(uv_loop_t));
    if (loop) {
      uv_loop_init(loop);
    }
    return loop;
}

void signal_handler(uv_signal_t *handle, int signum)
{
    printf("Signal received: %d\n", signum);
    uv_signal_stop(handle);
}

// two signal handlers in one loop
void thread1_worker(void *userp)
{
    uv_loop_t *loop1 = create_loop();

    uv_signal_t sig1a, sig1b;
    uv_signal_init(loop1, &sig1a);
    uv_signal_start(&sig1a, signal_handler, SIGUSR1); // receive CTRL+C

    uv_signal_init(loop1, &sig1b);
    uv_signal_start(&sig1b, signal_handler, SIGUSR1);

    uv_run(loop1, UV_RUN_DEFAULT);
}

void idle_cb1(uv_idle_t* handle) {
    printf("idle_cb1\n");
}

void idle_cb2(uv_idle_t* handle) {
    printf("idle_cb2\n");
}

// two signal handlers, each in its own loop
void thread2_worker(void *userp)
{
    uv_loop_t *loop2 = create_loop();
    uv_loop_t *loop3 = create_loop();

    uv_signal_t sig2;
    uv_signal_init(loop2, &sig2);
    uv_signal_start(&sig2, signal_handler, SIGUSR1);

    uv_signal_t sig3;
    uv_signal_init(loop3, &sig3);
    uv_signal_start(&sig3, signal_handler, SIGUSR1);

    // uv_idle_t idl_req1;
    // uv_idle_init(loop2, &idl_req1);
    // uv_idle_start(&idl_req1, idle_cb1);

    uv_idle_t idl_req2;
    uv_idle_init(loop3, &idl_req2);
    uv_idle_start(&idl_req2, idle_cb2);

    int i = 0, j = 0;
    while ((i = uv_run(loop2, UV_RUN_NOWAIT)) != 0 || (j = uv_run(loop3, UV_RUN_NOWAIT)) != 0) { //  只有在loop2中没有handle运行了，loop3才会运行
        // printf("i = %d, j = %d\n",i, j);
        sleep(1);
    }

    // uv_run(loop2, UV_RUN_NOWAIT);
    // uv_run(loop3, UV_RUN_NOWAIT);
}

int main()
{
    printf("PID %d\n", getpid());

    uv_thread_t thread1, thread2;

    uv_thread_create(&thread1, thread1_worker, 0);
    uv_thread_create(&thread2, thread2_worker, 0);

    uv_thread_join(&thread1);
    uv_thread_join(&thread2);
    return 0;
}
