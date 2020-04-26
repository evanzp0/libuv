#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#define BUF_SIZE 100

typedef struct {
    uv_udp_send_t req;
    uv_buf_t buf;
} udp_t;

void input_and_send(uv_udp_t * sock, struct sockaddr_in *serv_addr, char* msg) ;

void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
    memset(buf->base, 0, suggested_size);
}

void udp_send_cb(uv_udp_send_t* req, int status){
    udp_t *write_req_wrapper = (udp_t *)req;
    free(write_req_wrapper->buf.base);
    free(write_req_wrapper);
}

void recv_cb(uv_udp_t* sock, ssize_t nread, 
        const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags) {
    if (nread < 0) {
        fprintf(stderr, "Read error :%s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) sock, NULL); 
        free(buf->base);
        return;
    } else if (nread == 0) {
        free(buf->base); // 千万不能漏
        // uv_udp_recv_stop(sock); // 和 uv_close 同效
        return;
    }
    struct sockaddr_in *clnt_addr = (struct sockaddr_in *) addr;
    printf("Receive from client [%s : %d]: %s \n", inet_ntoa(clnt_addr->sin_addr), ntohs(clnt_addr->sin_port), buf->base);
    
    struct sockaddr_in *c_addr = malloc(sizeof(struct sockaddr_in));
    uv_ip4_addr("127.0.0.1", 14581, c_addr);
    input_and_send(sock, c_addr, buf->base);

    // free(buf->base);
}

int main () {
    uv_loop_t loop;
    uv_udp_t serv_sock;
    struct sockaddr_in addr;

    uv_loop_init(&loop);
    uv_udp_init(&loop, &serv_sock);
    
    uv_ip4_addr("0.0.0.0", 14580, &addr);
    uv_udp_bind(&serv_sock,(struct sockaddr *) &addr, UV_UDP_REUSEADDR);
    uv_udp_recv_start(&serv_sock, alloc_cb, recv_cb);
    printf("Listing at ( 0.0.0.0 : %d ) ...\n", 14580);

    // struct sockaddr_in c_addr;
    // uv_ip4_addr("127.0.0.1", 14581, &c_addr);
    // input_and_send(&serv_sock, &c_addr);

    uv_run(&loop, UV_RUN_DEFAULT);
    return 0;
}

void input_and_send(uv_udp_t * sock, struct sockaddr_in *serv_addr, char* msg) {
    // char *input_buf = (char *)malloc(BUF_SIZE);
    // memcpy(input_buf, msg, strlen(msg));
    char *input_buf = msg;

    udp_t *write_req_wrapper = malloc(sizeof(udp_t));
    write_req_wrapper->buf = uv_buf_init(input_buf, strlen(input_buf));
    uv_udp_send((uv_udp_send_t *) write_req_wrapper, sock, &write_req_wrapper->buf, 
            1, (struct sockaddr *) serv_addr, udp_send_cb);
    puts("sent to server!");
    // memset(write_req_wrapper, 0, sizeof(udp_t)); //  udp 只能异步调用，所以导致uv_run() 是产生 Segmentation fault: 11
}