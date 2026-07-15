#ifndef __WORKER_THREAD_H
#define __WORKER_THREAD_H
#include "common.h"
#include "http_parser.h"
#include "file_handler.h"
#include <sys/socket.h>
#include <pthread.h>
#include <fcntl.h>

typedef struct client_info_t{
    struct sockaddr_in client_addr;
    int client_fd;

}client_info_t;


void *handle_client(void *arg);

#endif // !__WORKER_THREAD_H
