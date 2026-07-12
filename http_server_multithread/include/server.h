#ifndef __SERVER_H
#define __SERVER_H

#include <sys/socket.h>
#include <pthread.h>
#include <fcntl.h>
#include "worker_thread.h"
#include "common.h"


void start_server(int port);

#endif // !__SERVER_H
