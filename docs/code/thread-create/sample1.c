#include <stdio.h>
#include <unistd.h>

#include <uv.h>

void worker(void *arg) {
  uv_thread_t mthread = *((uv_thread_t *) arg);
  uv_thread_t nthread = uv_thread_self();
  if(!uv_thread_equal(&mthread,&nthread)) {
      puts("not equal");
  }
}
int main(int argc, char **argv) {
  uv_thread_t mthread = uv_thread_self();
  uv_thread_t nthread;
  uv_thread_create(&nthread, worker, &mthread);
  uv_thread_join(&nthread);
}