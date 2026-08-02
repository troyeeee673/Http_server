#include "http_response.h"

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

// 错误响应
void build_error_response(int client_fd, int status_code, const char *status_msg) {
    const char *error_body = 
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head><meta charset=\"UTF-8\"><title>Error</title></head>\n"
        "<body>\n"
        "<h1>Error %d</h1>\n"
        "<p>%s</p>\n"
        "</body>\n"
        "</html>\n";
    
    char body[1024];
    snprintf(body, sizeof(body), error_body, status_code, status_msg);
    
    send_response_header(client_fd, status_code, status_msg, "text/html; charset=utf-8", strlen(body));
    send(client_fd, body, strlen(body), 0);
}

