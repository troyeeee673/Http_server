#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "logger.h"

static FILE *log_file = NULL;

// 初始化日志文件
void log_init(const char *file_path) {
    log_file = fopen(file_path, "a");   // 追加模式
    if (!log_file) {
        log_file = stdout;              // 打开失败则输出到终端
    }
}

// 核心写日志函数
static void log_write(const char *level, const char *format, va_list args) {
    if (!log_file) log_file = stdout;

    // 获取时间戳
    time_t now = time(NULL);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

    // 写入：[时间] [级别] 消息内容
    fprintf(log_file, "[%s] [%s] ", time_str, level);
    vfprintf(log_file, format, args);    // 可变参数输出
    fprintf(log_file, "\n");
    fflush(log_file);                    // 立即刷新到文件
}

// 对外接口：INFO 级别
void log_info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_write("INFO", format, args);
    va_end(args);
}

// 对外接口：ERROR 级别
void log_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_write("ERROR", format, args);
    va_end(args);
}

// 关闭日志文件
void log_close(void) {
    if (log_file && log_file != stdout) {
        fclose(log_file);
        log_file = NULL;
    }
}