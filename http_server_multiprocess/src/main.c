#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "config.h"
#include "logger.h"
#include "server.h"

int main(void) {
    //服务器配置
    server_config_t config;
    //加载服务器配置，初始化服务器
    load_default_config(&config);

    log_init(config.log_file);
    log_info("Starting HTTP server on port %d...", config.port);

    signal(SIGPIPE, SIG_IGN);

    start_server(config.port);

    log_close();
    return 0;
}
