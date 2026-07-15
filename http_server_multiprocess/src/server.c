#define _POSIX_C_SOURCE 199309L//添加特性宏
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "server.h"
#include "network.h"
#include "worker_thread.h"
#include "logger.h"

static volatile sig_atomic_t keep_running = 1;

void handle_signal(int sig) {
    (void)sig;
    keep_running = 0;
}

//初始化服务端，完成socket,bind,listen
int init_server_socket(int port)
{
    int server_fd;
    struct sockaddr_in addr;
    int opt = 1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket()");
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt()");
        close(server_fd);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind()");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("listen()");
        close(server_fd);
        return -1;
    }

    return server_fd;
}

void start_server(int port) {
    int fd;
    int server_fd = init_server_socket(port);
    if (server_fd < 0) {
        log_error("Failed to create socket");
        return;
    }

    // 改用 sigaction，更可靠
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;//设置信号处理函数
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    log_info("Server started on port %d", port);
    printf("Server listening on http://0.0.0.0:%d (Ctrl+C to stop)\n", port);

    //循环接收客户端请求
    while (keep_running) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        memset(&client_addr, 0, sizeof(client_addr));

        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0) {
            if (errno == EINTR) break;  // 被信号中断
            perror("accept()");
            continue;
        }

        client_info_t *client = malloc(sizeof(client_info_t));
        client->client_fd = client_fd;
        client->client_addr = client_addr;

        fd = fork();
        if(fd < 0)
        {
            perror("fork()");
            log_info("创建子进程失败");
            return;
        }
        if(fd == 0)
        {
            handle_client((void *)client);
            close(server_fd);//子进程关闭服务端监听文件描述符
            exit(0);//处理完毕之后退出
        }
        else if(fd > 0)
        {
            close(client_fd);//关闭客户端fd(不需要的文件描述符)
        }
    }


    close(server_fd);
    log_info("Server stopped");
    printf("Server stopped.\n");
}
