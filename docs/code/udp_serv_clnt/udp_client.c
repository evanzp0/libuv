#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#define DEFAULT_PORT 0
#define BUF_SIZE 100

typedef struct {
    uv_udp_send_t req;
    uv_buf_t buf;
} udp_t;

void input_and_send(uv_udp_t * sock, struct sockaddr_in *serv_addr);
void udp_send_cb(uv_udp_send_t* req, int status);
void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
    memset(buf->base, 0, suggested_size);
}

void on_send(uv_udp_send_t *req, int status) {
    if (status) {
        fprintf(stderr, "Send error %s\n", uv_strerror(status));
        return;
    }
}

void recv_cb(uv_udp_t* sock, ssize_t nread, 
        const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags) {
    if (nread < 0) {
        fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) sock, NULL);
        free(buf->base);
        return;
    } else if (nread == 0) {
        free(buf->base); // 千万不能漏
        return;
    }
        
    struct sockaddr_in *clnt_addr = (struct sockaddr_in *) addr;
    printf("Receive from client [%s : %d]: %s \n", inet_ntoa(clnt_addr->sin_addr), ntohs(clnt_addr->sin_port), buf->base);

    free(buf->base);

    input_and_send(sock, clnt_addr); 
}

int main () {
    uv_loop_t loop;
    uv_udp_t sock;
    struct sockaddr_in addr;

    uv_loop_init(&loop);
    uv_udp_init(&loop, &sock);

    // udp 没 bind 的话，uv_udp_send() 会自动调用 uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &addr) 进行绑定
    uv_ip4_addr("0.0.0.0", 14581, &addr);
    uv_udp_bind(&sock,(struct sockaddr *) &addr, UV_UDP_REUSEADDR);
    uv_udp_recv_start(&sock, alloc_cb, recv_cb);
    printf("Listing at ( 0.0.0.0 : %d ) ...\n", 14581);

    struct sockaddr_in serv_addr;
    uv_ip4_addr("127.0.0.1", 14580, &serv_addr);
    // uv_udp_connect(&sock, (struct sockaddr *) &serv_addr);
    input_and_send(&sock, &serv_addr); 

    uv_run(&loop, UV_RUN_DEFAULT);

    return 0;
}

void input_and_send(uv_udp_t * sock, struct sockaddr_in *serv_addr) {

    char *input_buf = (char *)malloc(BUF_SIZE);

    int n = 0;
    char *find = 0;
    printf("Input now (Press Q to quit): ");
    do {
        memset(input_buf, 0, BUF_SIZE);
        if(fgets(input_buf, BUF_SIZE, stdin)){
            if(strlen(input_buf) > 0 && (find = strchr(input_buf, '\n'))) {
                *find = '\0';
                if(('Q' == *input_buf || 'q' == *input_buf) && strlen(input_buf) == 1) {
                    uv_udp_recv_stop(sock);
                    return;
                }
            }
        }
        n = strlen(input_buf);
    } while (!n);

    udp_t *write_req_wrapper = malloc(sizeof(udp_t));
    write_req_wrapper->buf = uv_buf_init(input_buf, strlen(input_buf));
    uv_udp_send((uv_udp_send_t *) write_req_wrapper, sock, &write_req_wrapper->buf, 
            1, (struct sockaddr *) serv_addr, udp_send_cb);
    puts("sent to server!");
    // memset(write_req_wrapper, 0, sizeof(udp_t)); //  udp 只能异步调用，所以导致uv_run() 是产生 Segmentation fault: 11

}

void udp_send_cb(uv_udp_send_t* req, int status){
    udp_t *write_req_wrapper = (udp_t *)req;
    free(write_req_wrapper->buf.base);
    free(write_req_wrapper);
}