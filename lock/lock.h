#ifndef _MY_LOCK
#define _MY_LOCK

#include <sys/types.h>

typedef struct {
    char*   lock_file_r;
    char*   lock_file_w;
    int     fd;
} MY_FILE;

MY_FILE* lock_open(char* path, int oflag, int mode);
int lock_close(MY_FILE* my_file);
ssize_t lock_read(MY_FILE* my_file, void* buf, size_t nbyte);
ssize_t lock_write(MY_FILE* my_file, void* buf, size_t nbyte);

#endif
