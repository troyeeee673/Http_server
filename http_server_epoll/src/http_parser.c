#include "http_parser.h"

// 获取指定请求头
const char *get_header_value(const http_request_t *request, const char *name) {
    for (int i = 0; i < request->header_count; i++) {
        if (strcasecmp(request->headers[i].name, name) == 0) {
            return request->headers[i].value;
        }
    }
    return NULL;
}

// URL 解码（%20 → 空格）
static void url_decode(char *url) {
    char decoded[MAX_URL_LEN] = {0};
    int i = 0, j = 0;

    while (url[i] && j < MAX_URL_LEN - 1) {
        if (url[i] == '%' && url[i+1] && url[i+2]) {
            int hex;
            sscanf(url + i + 1, "%2x", &hex);
            decoded[j++] = (char)hex;
            i += 3;
        } else if (url[i] == '+') {
            decoded[j++] = ' ';   // POST 表单中 + 表示空格
            i++;
        } else {
            decoded[j++] = url[i++];
        }
    }

    decoded[j] = '\0';
    strcpy(url, decoded);
}


void parse_http_request(const char *raw_data, http_request_t *request) {
    const char *ptr = raw_data;
    char *line_end;

    // 初始化
    memset(request, 0, sizeof(http_request_t));

    // ---------- 1. 解析请求行: 方法 请求URL Http版本 ----------
    // 找到第一行结束位置
    line_end = strstr(ptr, "\r\n");
    if (!line_end) return;

    // 提取请求行
    char request_line[512] = {0};
    size_t line_len = line_end - ptr;//请求行长度
    // 限制长度
    if (line_len > sizeof(request_line) - 1) line_len = sizeof(request_line) - 1;
    strncpy(request_line, ptr, line_len);//拷贝请求行数据到request_line

    // 解析 方法 请求URL Http版本
    sscanf(request_line, "%7s %255s %15s",
           request->method, request->url, request->version);

    ptr = line_end + 2;  // 跳过 \r\n，到下一行

    // ---------- 2. 解析请求头 ----------
    request->header_count = 0;

    while (*ptr && strncmp(ptr, "\r\n", 2) != 0) {
        if (request->header_count >= MAX_HEADERS) break;

        line_end = strstr(ptr, "\r\n");
        if (!line_end) break;

        // 解析 Name: Value
        char header_line[512] = {0};
        line_len = line_end - ptr;
        if (line_len > sizeof(header_line) - 1) line_len = sizeof(header_line) - 1;
        strncpy(header_line, ptr, line_len);

        // 分割 name 和 value
        char *colon = strchr(header_line, ':');
        if (colon) {
            *colon = '\0';                              // name 结束
            strncpy(request->headers[request->header_count].name, header_line, 63);

            char *value = colon + 1;
            while (*value == ' ') value++;              // 跳过空格
            strncpy(request->headers[request->header_count].value, value, 255);

            request->header_count++;
        }

        ptr = line_end + 2;  // 跳过 \r\n
    }

    // ---------- 3. 跳过空行 ----------
    if (strncmp(ptr, "\r\n", 2) == 0) {
        ptr += 2;
    }

    // ---------- 4. 解析请求体 ----------
    if (*ptr) {
        strncpy(request->body, ptr, MAX_BODY_LEN - 1);
        request->body_len = strlen(request->body);
    }

    // ---------- 5. URL 解码 ----------
    url_decode(request->url);
}

