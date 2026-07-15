#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    int port;               //端口
    char root_dir[256];     //服务端文件保存目录，客户端请求数据时，会从这里查看是否有请求文件
    int max_threads;        //最大线程数
    char log_file[256];     //日志文件目录
} server_config_t;

void load_default_config(server_config_t *config);

#endif
