
#include "file.h"

extern inline int file_open_r(const char *const file_path);

extern inline int file_open_a(const char *const file_path);

extern inline int file_open_w(const char *const file_path);

extern inline int file_close(int fd);

string *file_read_to_string(int fd) {
    struct stat sb;
    if (fstat(fd, &sb) == -1) return NULL;
    string *s = string_init(sb.st_size);
    s->len = sb.st_size;
    if (read(fd, s->buffer, sb.st_size) != sb.st_size) {
        string_free(s);
        return NULL;
    }
    return s;
}
