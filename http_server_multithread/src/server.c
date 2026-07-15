#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "server.h"
#include "network.h"
#include "worker_thread.h"
#include "logger.h"

#define MAX_EVENTS 1024  // 最大监听事件数

static volatile sig_atomic_t keep_running = 1;

void handle_signal(int sig)
{
    (void)sig;
    keep_running = 0;
}

int init_server_socket(int port)
{
    int server_fd;
    struct sockaddr_in addr;
    int opt = 1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket()");
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt()");
        close(server_fd);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind()");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, SOMAXCONN) < 0)
    {
        perror("listen()");
        close(server_fd);
        return -1;
    }

    return server_fd;
}

void start_server(int port)
{
    int server_fd = init_server_socket(port);
    if (server_fd < 0)
    {
        log_error("Failed to create socket");
        return;
    }

    // 信号处理
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    log_info("Server started on port %d", port);
    printf("Server listening on http://0.0.0.0:%d (Ctrl+C to stop)\n", port);

    // ---------- 1. 创建 epoll 实例 ----------
    int epollfd = epoll_create1(0);
    if (epollfd < 0) {
        perror("epoll_create1()");
        close(server_fd);
        return;
    }

    // ---------- 2. 把 server_fd 加入监听 ----------
    struct epoll_event ev;
    ev.events = EPOLLIN;        // 监听可读事件（新连接到来）
    ev.data.fd = server_fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server_fd, &ev) < 0) {
        perror("epoll_ctl()");
        close(epollfd);
        close(server_fd);
        return;
    }

    // ---------- 3. 事件数组（输出用） ----------
    struct epoll_event events[MAX_EVENTS];

    // ---------- 4. 主循环 ----------
    while (keep_running)
    {
        // 等待事件发生（-1 表示阻塞等待）
        int nready = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nready < 0) {
            if (errno == EINTR) break;  // 信号中断，退出
            perror("epoll_wait()");
            continue;
        }

        // ---------- 5. 遍历就绪的事件 ----------
        for (int i = 0; i < nready; i++) {
            int fd = events[i].data.fd;

            if (fd == server_fd) {
                // === 新连接到来 ===
                struct sockaddr_in client_addr;
                socklen_t addr_len = sizeof(client_addr);
                memset(&client_addr, 0, sizeof(client_addr));

                int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
                if (client_fd < 0) {
                    if (errno == EINTR) goto cleanup;
                    perror("accept()");
                    continue;
                }

                // 把客户端 fd 也加入 epoll 监听
                ev.events = EPOLLIN;
                ev.data.fd = client_fd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
                    perror("epoll_ctl: client_fd");
                    close(client_fd);
                    continue;
                }

                log_info("新客户端连接 fd=%d", client_fd);

            } else {
                // === 客户端数据到达 ===
                // 创建线程处理（和之前多线程方式一样）
                client_info_t *client = malloc(sizeof(client_info_t));
                client->client_fd = fd;
                
                // 从 epoll 中移除（线程会接管）
                epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);

                pthread_t tid;
                if (pthread_create(&tid, NULL, handle_client, client) != 0) {
                    perror("pthread_create()");
                    close(fd);
                    free(client);
                } else {
                    pthread_detach(tid);
                }
            }
        }
    }

cleanup:
    close(epollfd);
    close(server_fd);
    log_info("Server stopped");
    printf("Server stopped.\n");
}