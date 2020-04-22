#include <stdio.h>
#include <uv.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 128

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_request_t;

void connect_cb(uv_connect_t* req, int status);
void close_cb(uv_handle_t* handle);
void write_cb(uv_write_t* req, int status);
void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
void input_and_send(uv_stream_t * handle);

int main(int argc, char **argv) {
    uv_loop_t *loop = uv_default_loop();
    uv_tcp_t sock;
    struct sockaddr_in dest_addr;

    uv_tcp_init(loop, &sock);
    uv_connect_t *connect_req = (uv_connect_t *)malloc(sizeof(uv_connect_t));
    uv_ip4_addr(argv[1], atoi(argv[2]), &dest_addr);
    uv_tcp_connect(connect_req, &sock, (const struct sockaddr *)&dest_addr, connect_cb);

    uv_run(loop, UV_RUN_DEFAULT);
}

void connect_cb(uv_connect_t* connect_req, int status) {
    if (status < 0) { // error!
        fprintf(stderr, "Connection error: %s\n", uv_strerror(status));
        
        uv_close((uv_handle_t*) connect_req->handle, close_cb);
        free(connect_req);
        return;
    }

    puts("Connected!");

    input_and_send((uv_stream_t *)connect_req->handle);
}

void input_and_send(uv_stream_t * handle) {
    char *input_buf = (char *)malloc(BUF_SIZE);
    memset(input_buf, 0, BUF_SIZE);
    printf("Input now: ");
    if(fgets(input_buf, BUF_SIZE, stdin)){
        input_buf[strlen(input_buf) - 1] = 0;
    }
    puts("Sendto server ! ");

    write_request_t *write_req = (write_request_t *)malloc(sizeof(write_request_t));
    write_req->buf = uv_buf_init(input_buf, strlen(input_buf));

    uv_write((uv_write_t *)write_req, handle, &write_req->buf, sizeof(write_req->buf), write_cb);
}

void close_cb(uv_handle_t* sock) {
    free(sock);
}

void write_cb(uv_write_t* wr_req, int status) {
    if (status) { // status will be 0 in case of success, < 0 otherwise.
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    uv_read_start(wr_req->handle, alloc_cb, read_cb);

    write_request_t *wr = (write_request_t*) wr_req;
    free(wr->buf.base);
    free(wr);
}

void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    *buf = uv_buf_init((char*) malloc(suggested_size), suggested_size);
}

void read_cb(uv_stream_t* sock, ssize_t nread, const uv_buf_t* buf) {
    if (nread < 0) {
        fprintf(stderr, "error echo_read");
        uv_close((uv_handle_t *)sock, close_cb);

        return;
    }
    puts(buf->base);

    input_and_send(sock);
}