#include <stdio.h>
#include <unistd.h>

#include <uv.h>

static uv_once_t once = UV_ONCE_INIT;
static void init_once(void) {
  printf("init once\n");
}
void worker(void *arg) {
  uv_once(&once, init_once);
  uv_thread_t tmp = uv_thread_self();
  printf("thread id:%lu\n", tmp->__sig);
}
int main(int argc, char **argv) {
  uv_thread_t nthread1,nthread2;
  uv_thread_create(&nthread1, worker, NULL);
  uv_thread_create(&nthread2, worker, NULL);
  uv_thread_join(&nthread1);
  uv_thread_join(&nthread2);
}