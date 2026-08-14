#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "worker_thread.h"
#include "http_parser.h"
#include "file_handler.h"
#include <sys/sendfile.h>

void *handle_client(void *arg)
{
    client_info_t *client_info = (client_info_t *)arg;
    int client_fd = client_info->client_fd;

    // 获取客户端地址和端口
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_info->client_addr.sin_addr, ip, INET_ADDRSTRLEN);
    int port = ntohs(client_info->client_addr.sin_port);
    printf("客户端已连接：%s:%d\n", ip, port);

    // 接收原始数据
    char data_receive[BUFFER_SIZE] = {0};
    int ret = recv(client_fd, data_receive, sizeof(data_receive) - 1, 0);
    if (ret <= 0) {
        printf("客户端断开：%s:%d\n", ip, port);
        close(client_fd);
        free(client_info);    // 释放内存
        return NULL;
    }
    data_receive[ret] = '\0';

    // 解析请求
    http_request_t request;
    memset(&request, 0, sizeof(request));
    parse_http_request(data_receive, &request);

    printf("请求：%s %s\n", request.method, request.url);

    // 处理请求并直接发送文件内容
    handle_file_request(client_fd, &request);

    // 关闭连接，释放资源
    close(client_fd);
    free(client_info);
    
    return NULL;
}