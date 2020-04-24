#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#define BUF_SIZE 1024

void open_cb(uv_fs_t* req);
void read_cb(uv_fs_t* req);
void close_cb(uv_fs_t* req);
void write_cb(uv_fs_t* fs_write_req);

int main(int argc, char **argv) {
    uv_loop_t loop; 
    uv_loop_init(&loop);

    uv_fs_t fs_open_req;
    
    // 和 uv_tcp_t 不同，uv_fs_t 没有类似 uv_tcp_init() 这样的初始化函数，因为进行 uv_tcp_init() 为了获得 socket fd，然后才能band、listen 和 connect。
    // 而 文件是在打开时生成 file fd，你也可以把 uv_fs_open() 看成是 uv_fs_t 的初始化。
    uv_fs_open(&loop, &fs_open_req, "/Users/zhangevan/Documents/workspace/libuv/docs/code/uvcat/hello.txt", O_RDONLY, 0, open_cb); 
    // uv_fs_read(&loop, &fs_read_req)
    printf("loop address: %p\n", &loop);

    uv_run(&loop, UV_RUN_DEFAULT);
    uv_loop_close(&loop);
}

void open_cb(uv_fs_t* fs_open_req) {
    if (fs_open_req->result < 0) { // error // fs_open_req->file?
        fprintf(stderr, "error opening file: %s\n", uv_strerror((int)fs_open_req->result));
        uv_fs_close(fs_open_req->loop, fs_open_req, fs_open_req->file, close_cb);
    } else { 
        uv_fs_t *fs_read_req = malloc(sizeof(uv_fs_t));
        uv_buf_t buf = uv_buf_init(malloc(BUF_SIZE), BUF_SIZE);
        fs_read_req->bufs = &buf;

        printf("loop address: %p\n", fs_open_req->loop);
        // uv_fs_read(fs_open_req->loop, fs_read_req, fs_open_req->file, fs_read_req->bufs, 1, -1, read_cb); // fs_open_req->file == 0 当心这个坑
        uv_fs_read(fs_open_req->loop, fs_read_req, fs_open_req->result, fs_read_req->bufs, 1, -1, read_cb);
    }
    // 回收 fs_open_req 内存，因为虽然它是局部变量，但是在后续的回调过程中并没啥用
    // uv_fs_req_cleanup(fs_open_req);
}

void read_cb(uv_fs_t* fs_read_req) {
    printf("loop address: %p\n", fs_read_req->loop);

    if (fs_read_req->result < 0) { // error 
        fprintf(stderr, "Write error: %s\n", uv_strerror((int) fs_read_req->result));
    } else if (fs_read_req->result == 0) { // 没有数据了
        uv_fs_close(fs_read_req->loop, fs_read_req, fs_read_req->file, close_cb);
        return ;
    }
    printf("The file fd: %d, is read! \n", fs_read_req->file);
    uv_fs_t *fs_write_t = malloc(sizeof(uv_fs_t));
    uv_fs_write(fs_read_req->loop, fs_write_t, 1, fs_read_req->bufs, 1, -1, write_cb);

    // 回收 fs_read_req 内存
    // uv_fs_req_cleanup(fs_read_req);
}

void write_cb(uv_fs_t* fs_write_req) {
    printf("loop address: %p\n", fs_write_req->loop);

    if (fs_write_req->result < 0) {
        fprintf(stderr, "Write error: %s\n", uv_strerror((int) fs_write_req->result));
    }

    printf("The file fd: %d, is writed! \n", fs_write_req->file);
    uv_fs_t *fs_read_req = malloc(sizeof(uv_fs_t));
    uv_buf_t buf = uv_buf_init(malloc(BUF_SIZE), BUF_SIZE);
    fs_read_req->bufs = &buf;

    // 读写循环
    // 因为 uv_fs_t 是 request 级别监听器，只在一个事件循环里起作用，如果不在再次注册，程序就会退出。
    // 如果是 uv_tcp_t 那样的 handle 级别的监听器，在整个 loop 循环中都会存在，就只需要在 loop 中注册一次即可反复被调用
    uv_fs_read(fs_write_req->loop, fs_read_req, fs_write_req->file, fs_read_req->bufs, 1, -1, read_cb);

    // 回收 fs_write_req 内存
    uv_fs_req_cleanup(fs_write_req);
}

void close_cb(uv_fs_t* fs_close_req) {
    printf("loop address: %p\n", fs_close_req->loop);

    printf("The file fd: %d, is closed! \n", fs_close_req->file);
}
