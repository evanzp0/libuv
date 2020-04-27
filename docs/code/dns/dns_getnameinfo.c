#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

void getnameinfo_cb(uv_getnameinfo_t* req, int status, const char* hostname, const char* service) {
    if (status) {
        fprintf(stderr, "getaddrinfo callback error: %s\n", uv_err_name(status));
        return ;
    }

    fprintf(stderr, "%s\n", hostname);
}

int main() {
    uv_loop_t loop;
    uv_getnameinfo_t resolver;

    uv_loop_init(&loop);
    
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = inet_addr("162.213.39.42");
    addr.sin_family = PF_INET;
    // addr.sin_port = htons(80);

    int r = uv_getnameinfo(&loop, &resolver, getnameinfo_cb, (struct sockaddr *) &addr, 0);
    if (r) {
        fprintf(stderr, "getaddrinfo callback error: %s\n", uv_err_name((ssize_t)r));
        return 1;
    }
    return uv_run(&loop, UV_RUN_DEFAULT);
}