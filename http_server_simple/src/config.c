#include <string.h>
#include "config.h"

void load_default_config(server_config_t *config) {
    config->port = 8080;
    strcpy(config->root_dir, "www");
    config->max_threads = 100;
    strcpy(config->log_file, "logs/server.log");
}
