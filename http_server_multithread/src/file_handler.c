#include "file_handler.h"

// 前向声明
static void serve_file(const char *file_path, char *response);
static void serve_directory_listing(const char *dir_path, const char *url, char *response);

void handle_file_request(const http_request_t *request, char *response) {
    char file_path[MAX_PATH];
    struct stat file_stat;

    // 安全检查
    if (strstr(request->url, "..") || strstr(request->url, "//")) {
        build_error_response(403, "Forbidden", response);
        return;
    }

    // 构建文件路径
    if (strcmp(request->url, "/") == 0) {
        snprintf(file_path, sizeof(file_path), "%s/index.html", WWW_ROOT);
    } else {
        snprintf(file_path, sizeof(file_path), "%s%s", WWW_ROOT, request->url);
    }

    log_info("Request file: %s", file_path);

    // 检查文件是否存在
    if (stat(file_path, &file_stat) < 0) {
        build_error_response(404, "File Not Found", response);
        return;
    }

    // 目录处理
    if (S_ISDIR(file_stat.st_mode)) {
        // 检查目录下是否有 index.html
        char index_path[MAX_PATH];
        snprintf(index_path, sizeof(index_path), "%s/index.html", file_path);

        if (stat(index_path, &file_stat) == 0 && S_ISREG(file_stat.st_mode)) {
            serve_file(index_path, response);
        } else {
            serve_directory_listing(file_path, request->url, response);
        }
        return;
    }

    // 返回文件
    serve_file(file_path, response);
}

static void serve_file(const char *file_path, char *response) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        build_error_response(404, "File Not Found", response);
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size > 10 * 1024 * 1024) {
        fclose(file);
        build_error_response(413, "File Too Large", response);
        return;
    }

    char *file_data = malloc(file_size + 1);
    if (!file_data) {
        fclose(file);
        build_error_response(500, "Memory Allocation Failed", response);
        return;
    }

    fread(file_data, 1, file_size, file);
    file_data[file_size] = '\0';
    fclose(file);

    build_file_response(file_path, file_data, file_size, response);
    free(file_data);
}

static void serve_directory_listing(const char *dir_path, const char *url, char *response) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        build_error_response(500, "Cannot Open Directory", response);
        return;
    }

    char html[MAX_HTML];
    int offset = snprintf(html, sizeof(html),
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "<meta charset=\"UTF-8\">\n"
        "<title>Index of %s</title>\n"
        "<style>\n"
        "body { font-family: Arial, sans-serif; margin: 20px; }\n"
        "h1 { color: #333; }\n"
        "table { border-collapse: collapse; width: 100%%; }\n"
        "td { padding: 8px 12px; border-bottom: 1px solid #eee; }\n"
        "tr:hover { background: #f5f5f5; }\n"
        "a { color: #0366d6; text-decoration: none; }\n"
        "a:hover { text-decoration: underline; }\n"
        "</style>\n"
        "</head>\n"
        "<body>\n"
        "<h1>Index of %s</h1>\n"
        "<table>\n",
        url, url);

    if (strcmp(url, "/") != 0) {
        offset += snprintf(html + offset, sizeof(html) - offset,
            "<tr><td colspan=\"3\"><a href=\"../\">📁 ..</a></td></tr>\n");
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) == 0) {
            const char *icon = S_ISDIR(st.st_mode) ? "📁" : "📄";
            char time_str[64];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", localtime(&st.st_mtime));

            char size_str[32] = "-";
            if (S_ISREG(st.st_mode)) {
                if (st.st_size < 1024)
                    snprintf(size_str, sizeof(size_str), "%ld B", st.st_size);
                else if (st.st_size < 1024 * 1024)
                    snprintf(size_str, sizeof(size_str), "%.1f KB", st.st_size / 1024.0);
                else
                    snprintf(size_str, sizeof(size_str), "%.1f MB", st.st_size / (1024.0 * 1024.0));
            }

            offset += snprintf(html + offset, sizeof(html) - offset,
                "<tr>"
                "<td>%s</td>"
                "<td><a href=\"%s%s\">%s</a></td>"
                "<td>%s</td>"
                "<td>%s</td>"
                "</tr>\n",
                icon, entry->d_name, S_ISDIR(st.st_mode) ? "/" : "",
                entry->d_name, time_str, size_str);
        }
    }

    closedir(dir);

    snprintf(html + offset, sizeof(html) - offset,
        "</table>\n</body>\n</html>\n");

    build_response(200, "text/html; charset=utf-8", html, strlen(html), response);
}

const char *get_mime_type(const char *file_path) {
    const char *ext = strrchr(file_path, '.');
    if (!ext) return "application/octet-stream";

    if (strcasecmp(ext, ".html") == 0 || strcasecmp(ext, ".htm") == 0)
        return "text/html; charset=utf-8";
    if (strcasecmp(ext, ".css") == 0)   return "text/css";
    if (strcasecmp(ext, ".js") == 0)    return "application/javascript";
    if (strcasecmp(ext, ".json") == 0)  return "application/json";
    if (strcasecmp(ext, ".xml") == 0)   return "application/xml";
    if (strcasecmp(ext, ".jpg") == 0 || strcasecmp(ext, ".jpeg") == 0)
        return "image/jpeg";
    if (strcasecmp(ext, ".png") == 0)   return "image/png";
    if (strcasecmp(ext, ".gif") == 0)   return "image/gif";
    if (strcasecmp(ext, ".svg") == 0)   return "image/svg+xml";
    if (strcasecmp(ext, ".ico") == 0)   return "image/x-icon";
    if (strcasecmp(ext, ".mp3") == 0)   return "audio/mpeg";
    if (strcasecmp(ext, ".mp4") == 0)   return "video/mp4";
    if (strcasecmp(ext, ".pdf") == 0)   return "application/pdf";
    if (strcasecmp(ext, ".txt") == 0)   return "text/plain; charset=utf-8";

    return "application/octet-stream";
}
