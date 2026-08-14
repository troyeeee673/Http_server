#ifndef __FILE_HANDLER_H
#define __FILE_HANDLER_H
#include "http_parser.h"
#include "http_response.h"
#include <sys/stat.h>
#include <dirent.h>
#include "common.h"
#include "logger.h"

#define WWW_ROOT    "www"          // 静态文件根目录
#define MAX_PATH    1024
#define MAX_HTML    16384


void handle_file_request(int client_fd, const http_request_t *request);
const char *get_mime_type(const char *file_path);

#endif