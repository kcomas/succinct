
#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "string.h"

inline int file_open_r(const char *const file_path) {
    return open(file_path, O_RDONLY);
}

inline int file_open_a(const char *const file_path) {
    return open(file_path, O_WRONLY | O_APPEND | O_CREAT);
}

inline int file_open_w(const char *const file_path) {
    return open(file_path, O_WRONLY | O_CREAT);
}

inline int file_close(int fd) {
    return close(fd);
}

string *file_read_to_string(int fd);
