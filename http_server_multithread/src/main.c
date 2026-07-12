#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "config.h"
#include "logger.h"
#include "server.h"

int main(void) {
    server_config_t config;
    load_default_config(&config);

    log_init(config.log_file);
    log_info("Starting HTTP server on port %d...", config.port);

    signal(SIGPIPE, SIG_IGN);

    start_server(config.port);

    log_close();
    return 0;
}
