#ifndef __HTTP_PARSER_H
#define __HTTP_PARSER_H

#include "common.h"
#define MAX_HEADERS     20
#define MAX_URL_LEN     256
#define MAX_METHOD_LEN  8
#define MAX_BODY_LEN    4096

//请求头
typedef struct {
    char name[64];
    char value[256];
} http_header_t;

//请求体
typedef struct {
    char method[MAX_METHOD_LEN];       // 请求类型：GET/POST/HEAD
    char url[MAX_URL_LEN];             // 请求的url:/index.html
    char version[16];                  // HTTP版本：HTTP/1.1
    http_header_t headers[MAX_HEADERS];
    int header_count;
    char body[MAX_BODY_LEN];
    int body_len;
} http_request_t;

void parse_http_request(const char *raw_data, http_request_t *request);


#endif // !__HTTP_PARSER_H
