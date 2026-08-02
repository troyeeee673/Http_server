#ifndef __HTTP_RESPONSE_H
#define __HTTP_RESPONSE_H
#include "http_parser.h"
#include <time.h>
#include "common.h"
#include "file_handler.h"

#define RESPONSE_SIZE   8192
#define BUFFER_SIZE     4096

// 通用响应构建
void build_response(int status_code, const char *content_type,
                    const char *body, int body_len, char *response);

// 错误响应
void build_error_response(int client_fd, int status_code, const char *status_msg);

// 文件响应（自动判断 MIME 类型）
void build_file_response(const char *file_path, const char *data,
                         int data_len, char *response);

#endif