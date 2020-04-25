#include <stdio.h>
#include <stdlib.h>
#include <uv.h>
#include <unistd.h>

#define BUF_SIZE 1024

void open_cb(uv_fs_t* req);
void read_cb(uv_fs_t* req);
void close_cb(uv_fs_t* req);
void write_cb(uv_fs_t* fs_write_req);

// uv_fs_t wrap object
typedef struct {
    uv_fs_t req;
    uv_buf_t buf;
    int file;
} fs_t;

int main(int argc, char **argv) {
    uv_loop_t loop; 
    uv_loop_init(&loop);

    uv_fs_t fs_open_req;

    // 和 uv_tcp_t 不同，uv_fs_t 没有类似 uv_tcp_init() 这样的初始化函数，因为进行 uv_tcp_init() 为了获得 socket fd，然后才能band、listen 和 connect。
    // 而文件是在打开时生成 file fd 的，你也可以把 uv_fs_open() 看成是 uv_fs_t 的初始化。
    // 另外 uv_fs_t 是 request，而 uv_tcp_t 是 handle
    uv_fs_open(&loop, &fs_open_req, "/Users/zhangevan/Documents/workspace/libuv/docs/code/uvcat/hello.txt", O_RDONLY, 0, open_cb); 
    // uv_fs_read(&loop, &fs_read_req)
    printf("loop address: %p\n", &loop);

    uv_run(&loop, UV_RUN_DEFAULT);
    uv_loop_close(&loop);
}

void open_cb(uv_fs_t* fs_open_req) {

    if (fs_open_req->result < 0) { // error // fs_open_req->file?
        fprintf(stderr, "error opening file: %s\n", uv_strerror((int)fs_open_req->result));
        uv_fs_close(fs_open_req->loop, fs_open_req, fs_open_req->file, NULL);
    } else { 
        fs_t *fs_read_wrapper = malloc(sizeof(fs_t));
        fs_read_wrapper->buf = uv_buf_init(malloc(BUF_SIZE), BUF_SIZE);

        // uv_fs_read(fs_open_req->loop, fs_read_req, fs_open_req->file, fs_read_req->bufs, 1, -1, read_cb); 
        /*
         * 上面被注释的 us_fs_read() 有两个问题：
         * 1、fs_open_req->file == 0 当心这个坑
         * 2、fs_read_req->bufs 不能直接作为参数，因为在 us_fs_read() 中会进行这个成员的内部操作, 这里可以使用 wrap object 技巧，比全局变量更好
         */
        int e = uv_fs_read(fs_open_req->loop, (uv_fs_t *)fs_read_wrapper, fs_open_req->result, &fs_read_wrapper->buf, 1, -1, read_cb);
    }
    // 回收 fs_open_req 内存，因为虽然它是局部变量，但是在后续的回调过程中并没啥用
    uv_fs_req_cleanup(fs_open_req);
}

void read_cb(uv_fs_t* fs_read_req) {
    sleep(1);
    fs_t *rt = (fs_t*)fs_read_req;

    if (fs_read_req->result < 0) { // error 
        fprintf(stderr, "Write error: %s\n", uv_strerror((int) fs_read_req->result));
        uv_fs_close(fs_read_req->loop, (uv_fs_t* )rt, fs_read_req->file, NULL);

    } else if (fs_read_req->result == 0) { // 没有数据了
        uv_fs_close(fs_read_req->loop, (uv_fs_t* )rt, fs_read_req->file, NULL);

    } else {
        printf("The file fd: %d, is read! \n", fs_read_req->file);
        fs_t *write_fs_wrapper = malloc(sizeof(fs_t));
        write_fs_wrapper->file = rt->req.file;
        uv_fs_write(fs_read_req->loop, (uv_fs_t *) write_fs_wrapper, 1, &rt->buf, 1, -1, write_cb);
    }

    // 回收 fs_read_req 内存
    free(rt->buf.base);
    uv_fs_req_cleanup(&rt->req);
    free(rt);
}

void write_cb(uv_fs_t* fs_write_req) {
    fs_t * wt = (fs_t *) fs_write_req;

    if (fs_write_req->result < 0) {
        fprintf(stderr, "Write error: %s\n", uv_strerror((int) fs_write_req->result));
        uv_fs_close(fs_write_req->loop, (uv_fs_t* )wt, wt->file, NULL);
    } else {
        printf("The file fd: %d, is writed! \n", fs_write_req->file);
        fs_t *fs_read_wrapper = malloc(sizeof(fs_t));
        fs_read_wrapper->buf = uv_buf_init(malloc(BUF_SIZE), BUF_SIZE);

        // 读写循环
        // 因为 uv_fs_t 是 request 级别监听器，只在一个事件循环里起作用，如果不在再次注册，程序就会退出。
        // 如果是 uv_tcp_t 那样的 handle 级别的监听器，在整个 loop 循环中都会存在，就只需要在 loop 中注册一次即可反复被调用
        int e = uv_fs_read(fs_write_req->loop, (uv_fs_t *)fs_read_wrapper, wt->file, &fs_read_wrapper->buf, 1, -1, read_cb);
    }

    // 回收 fs_write_req 内存
    uv_fs_req_cleanup(&wt->req);
    free(wt);
}
