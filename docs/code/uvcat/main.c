#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <uv.h>

void on_read(uv_fs_t *req);

uv_fs_t open_req;
uv_fs_t read_req;
uv_fs_t write_req;

static char buffer[1024];

static uv_buf_t iov;

void on_write(uv_fs_t *req) {
    if (req->result < 0) {
        fprintf(stderr, "Write error: %s\n", uv_strerror((int)req->result));
    }
    else {
        uv_fs_read(uv_default_loop(), &read_req, open_req.result, &iov, 1, -1, on_read); // 读写循环
    }
}

void on_read(uv_fs_t *req) {
    if (req->result < 0) {
        fprintf(stderr, "Read error: %s\n", uv_strerror(req->result));
    }
    else if (req->result == 0) {
        uv_fs_t close_req;
        // synchronous
        uv_fs_close(uv_default_loop(), &close_req, open_req.result, NULL); // 读写退出点
    }
    else if (req->result > 0) { // 如果有数据则输出到 stdout
        iov.len = req->result;
        uv_fs_write(uv_default_loop(), &write_req, 1, &iov, 1, -1, on_write);  // flag = -1, 表示使用当前位移
    }
}

void on_open(uv_fs_t *req) {
    // The request passed to the callback is the same as the one the call setup
    // function was passed.
    assert(req == &open_req);
    if (req->result >= 0) { // req->result >= 0 表示 file_fd；req->result < 0 表示有错误
        iov = uv_buf_init(buffer, sizeof(buffer)); // 设置读取缓存，用完后要释放
        // 从指定文件中读取数据到 iov 缓存
        uv_fs_read(uv_default_loop(), &read_req, req->result,
                   &iov, 1, -1, on_read); 
    }
    else {
        fprintf(stderr, "error opening file: %s\n", uv_strerror((int)req->result));
    }
}

int main(int argc, char **argv) {
    // 如果 cb (就是最后一个回调函数)不为空，则 open_req->path 会指向 malloc 的一个区域，
    // 该区域的内容复制了 argv[1] 的字符串
    uv_fs_open(uv_default_loop(), &open_req, "/Users/zhangevan/Documents/workspace/libuv/docs/code/uvcat/hello.txt", O_RDONLY, 0,  on_open);

    // 结束循环
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);

    // uv_fs_open() 对于 req->path 会用 malloc 分配内存，所以用完后需要 cleanup
    uv_fs_req_cleanup(&open_req);
    // uv_fs_read() 和 uv_fs_write() 对于 req->bufs 会用 malloc 分配内存，所以用完后需要 cleanup
    uv_fs_req_cleanup(&read_req);
    uv_fs_req_cleanup(&write_req);
    // uv_fs_req_close() 只是调用 close(fd)，不存在 malloc 分配，所以用完后不需要 cleanup
    return 0;
}
