#include "http_response.h"
#include <sys/socket.h>

int send_response_header(int client_fd, int status_code,
                         const char *status_msg,
                         const char *content_type,
                         size_t content_length) {
    char header[4096];
    int len = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        status_code, status_msg,
        content_type ? content_type : "text/html",
        content_length
    );

    ssize_t sent = 0;
    while (sent < len) {
        ssize_t n = send(client_fd, header + sent, len - sent, 0);
        if (n <= 0) {
            return -1;
        }
        sent += n;
    }
    return 0;
}

// ---------- 1. 通用响应构建 ----------
void build_response(int status_code, const char *content_type,
                    const char *body, int body_len, char *response) {
    
    // 状态码 → 描述
    const char *status_text;
    switch (status_code) {
        case 200: status_text = "OK";              break;
        case 206: status_text = "Partial Content"; break;
        case 301: status_text = "Moved Permanently"; break;
        case 302: status_text = "Found";           break;
        case 304: status_text = "Not Modified";    break;
        case 400: status_text = "Bad Request";     break;
        case 403: status_text = "Forbidden";       break;
        case 404: status_text = "Not Found";       break;
        case 405: status_text = "Method Not Allowed"; break;
        case 413: status_text = "Payload Too Large"; break;
        case 500: status_text = "Internal Server Error"; break;
        default:  status_text = "Unknown";         break;
    }

    // 获取 GMT 时间
    time_t now = time(NULL);
    char date_str[64];
    strftime(date_str, sizeof(date_str), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&now));

    // 构建：状态行 + 响应头 + 空行
    int offset = snprintf(response, RESPONSE_SIZE,
        "HTTP/1.1 %d %s\r\n"           // 状态行
        "Server: TinyHTTPServer/1.0\r\n"
        "Date: %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",                          // 空行（头部结束）
        status_code, status_text,
        date_str,
        content_type,
        body_len);

    // 拼接响应体
    if (body && body_len > 0) {
        memcpy(response + offset, body, body_len);
        offset += body_len;
    }
    
    response[offset] = '\0';  // 字符串结束
}

// ---------- 2. 错误响应 ----------
void build_error_response(int status_code, const char *message, char *response) {
    // 构建 HTML 错误页面
    char html[512];
    const char *status_text;

    switch (status_code) {
        case 400: status_text = "Bad Request";           break;
        case 403: status_text = "Forbidden";             break;
        case 404: status_text = "Not Found";             break;
        case 405: status_text = "Method Not Allowed";    break;
        case 500: status_text = "Internal Server Error"; break;
        default:  status_text = "Error";                 break;
    }

    snprintf(html, sizeof(html),
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head><meta charset=\"UTF-8\">"
        "<title>%d %s</title></head>\n"
        "<body style=\"font-family:Arial;text-align:center;margin-top:100px;\">\n"
        "<h1>%d %s</h1>\n"
        "<p>%s</p>\n"
        "<hr>\n"
        "<p><em>TinyHTTPServer/1.0</em></p>\n"
        "</body>\n"
        "</html>\n",
        status_code, status_text,
        status_code, status_text,
        message);

    build_response(status_code, "text/html; charset=utf-8", html, strlen(html), response);
}

// ---------- 3. 文件响应 ----------
void build_file_response(const char *file_path, const char *data,
                         int data_len, char *response) {
    const char *mime = get_mime_type(file_path);
    build_response(200, mime, data, data_len, response);
}