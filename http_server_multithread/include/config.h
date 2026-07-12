#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    int port;
    char root_dir[256];
    int max_threads;
    char log_file[256];
} server_config_t;

void load_default_config(server_config_t *config);

#endif
