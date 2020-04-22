#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <uv.h>

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

uv_loop_t *loop;
uv_pipe_t stdin_pipe;
uv_pipe_t stdout_pipe;
uv_pipe_t file_pipe;

/**
 * 在 stream 分配 buffer 空间时被调用
 */
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    // printf("suggested_size = %zu\n", suggested_size); // suggested_size = 64K
    // malloc 分配的空间，在 uv_buf_init() 中是赋值给了 buf->base，所以，在uv_read_start() 的回调函数 read_stdin()中使用完 buf 后，需要将 buf->base 释放。
    // buf 不是我们创建的数据，所以我们不用回收（谁创建的数据谁负责回收）
    *buf = uv_buf_init((char*) malloc(suggested_size), suggested_size);
}

void free_write_req(uv_write_t *req) {
    write_req_t *wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
}

/**
 * 回收分配的 requst 资源
 */
void on_stdout_write(uv_write_t *req, int status) {
    free_write_req(req);
}

void on_file_write(uv_write_t *req, int status) {
    free_write_req(req);
}

void write_data(uv_stream_t *dest, size_t size, uv_buf_t buf, uv_write_cb cb) {
    // 生成一个 write request (handle), 在 uv_write() 注册的 on_stdout_write 回调函数中，需要调用 free_write_req() 将它回收
    write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t)); // 回收点：1
    req->buf = uv_buf_init((char*) malloc(size), size); // 回收点：2，注意：这里 uv_buf_init() 返回一个局部变量，req->buf 对它进行了值COPY
    memcpy(req->buf.base, buf.base, size);  

    uv_write((uv_write_t*) req, (uv_stream_t*)dest, &req->buf, 1, cb); // req 是为了传给 cb 用的，&req->buf 才是用来输出数据的
}

void read_stdin(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    if (nread < 0){
        if (nread == UV_EOF){
            // end of file
            uv_close((uv_handle_t *)&stdin_pipe, NULL);
            uv_close((uv_handle_t *)&stdout_pipe, NULL);
            uv_close((uv_handle_t *)&file_pipe, NULL);
        }
    } else if (nread > 0) {
        write_data((uv_stream_t *)&stdout_pipe, nread, *buf, on_stdout_write);
        write_data((uv_stream_t *)&file_pipe, nread, *buf, on_file_write);
    }

    // 回收在 alloc_buffer() 中分配的 buf->base
    // OK to free buffer as write_data copies it.
    if (buf->base)
        free(buf->base);
}

int main(int argc, char **argv) {
    loop = uv_default_loop();

    uv_pipe_init(loop, &stdin_pipe, 0);
    uv_pipe_open(&stdin_pipe, 0); // 0：代表标准输入，将标准输入绑定到 stdin_pipe 上

    uv_pipe_init(loop, &stdout_pipe, 0);
    uv_pipe_open(&stdout_pipe, 1); // 1：代表标准输出，将标准输入绑定到 stdout_pipe 上
    
    uv_fs_t file_req;
    int fd = uv_fs_open(loop, &file_req, argv[1], O_CREAT | O_RDWR, 0644, NULL); // uv_fs_open() 被调用时，将 loop 和 file_req 绑定
    uv_pipe_init(loop, &file_pipe, 0);
    uv_pipe_open(&file_pipe, fd); // 将 file_fd 绑定到 file_pipe 上

    uv_read_start((uv_stream_t*)&stdin_pipe, alloc_buffer, read_stdin); // 非阻塞的读取 stdin_pipe

    uv_run(loop, UV_RUN_DEFAULT);
    return 0;
}
