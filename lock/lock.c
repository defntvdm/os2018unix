#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "lock.h"

/*
-1, если не удалось что либо сделать
0, если всё ок
*/
int get_lock_file(MY_FILE* mfile, struct stat* file_stat) {
    mfile->lock_file_r = (char*)malloc(35 * sizeof(char));
    if (!mfile->lock_file_r)
        return -1;
    mfile->lock_file_w = (char*)malloc(35 * sizeof(char));
    if (!mfile->lock_file_w) {
        free(mfile->lock_file_r);
        return -1;
    }
    sprintf(mfile->lock_file_r, "/tmp/%d_r.lck", file_stat->st_ino);
    sprintf(mfile->lock_file_w, "/tmp/%d_w.lck", file_stat->st_ino);
    return 0;
}

MY_FILE* lock_open(char* path, int oflag, int mode) {
    int fd = open(path, oflag, mode);
    if (fd < 0) {
        return NULL;
    }
    struct stat file_stat;
    stat(path, &file_stat);
    MY_FILE* mfile = (MY_FILE*)malloc(sizeof(MY_FILE));
    if (!mfile) {
        close(fd);
        return NULL;
    }
    if (get_lock_file(mfile, &file_stat) == -1) {
        free(mfile);
        close(fd);
        return NULL;
    }
    mfile->fd = fd;
    return mfile;
}

int lock_close(MY_FILE* mfile) {
    int fd = mfile->fd;
    free(mfile->lock_file_r);
    free(mfile->lock_file_w);
    free(mfile);
    return close(fd);
}

ssize_t lock_read(MY_FILE* mfile, void* buf, size_t nbyte) {
    int fd = open(mfile->lock_file_r, O_WRONLY | O_CREAT | O_EXCL, 0644);
    while (fd == -1) {
        usleep(1000);
        fd = open(mfile->lock_file_r, O_WRONLY | O_CREAT | O_EXCL, 0644);
    }
    char num[26];
    size_t len = sprintf(num, "%d", getpid());
    write(fd, num, len);
    write(fd, "\nread\n", 6);
    ssize_t result = read(mfile->fd, buf, nbyte);
    close(fd);
    unlink(mfile->lock_file_r);
    return result;
}

ssize_t lock_write(MY_FILE* mfile, void* buf, size_t nbyte) {
    int fd = open(mfile->lock_file_w, O_WRONLY | O_CREAT | O_EXCL, 0644);
    while (fd == -1) {
        usleep(1000);
        fd = open(mfile->lock_file_w, O_WRONLY | O_CREAT | O_EXCL, 0644);
    }
    char num[26];
    size_t len = sprintf(num, "%d", getpid());
    write(fd, num, len);
    write(fd, "\nwrite\n", 7);
    ssize_t result = write(mfile->fd, buf, nbyte);
    close(fd);
    unlink(mfile->lock_file_w);
    return result;
}
