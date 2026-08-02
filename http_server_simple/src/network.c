#include "network.h"

int socket_init()
{
    int sockfd;
    int ret = 0;
    int opt = 1;
    struct sockaddr_in addr;
    socklen_t len;
    // 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket()");
        ret = -EINVAL;
        goto fail_socket;
    }

    // 端口复用
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 服务器地址绑定
    memset(&addr, 0, sizeof(addr));
    len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(SERVER_PROT);

    ret = bind(sockfd, (struct sockaddr *)&addr, len);
    if (ret < 0)
    {
        ret = -EINVAL;
        perror("bind()");
        goto fail_bind;
    }

    // 监听
    ret = listen(sockfd, MAX_CONNECTION);
    if (ret < 0)
    {
        perror("listen()");
        ret = -EINVAL;
        goto fail_listen;
    }
    // 接收请求
    start_server(sockfd);

    return 0;

fail_listen:
fail_bind:
fail_socket:
    return ret;
}
