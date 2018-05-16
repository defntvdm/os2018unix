#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "lock.h"

int main(int argc, char** argv) {
    if (argc != 4) {
        printf("Usage: %s user password file\n", argv[0]);
        return 1;
    }
    int buf_size = strlen(argv[1]) + strlen(argv[2]) + 4;
    char* buf = (char*)malloc(buf_size * sizeof(char));
    if (!buf) {
        perror("Can't allocate memory for buffer");
        return 1;
    }
    sprintf(buf, "%s %s\n", argv[1], argv[2]);
    MY_FILE* file = lock_open(argv[3], O_WRONLY | O_CREAT | O_APPEND, 0644);
    printf("Start writing\n");
    lock_write(file, buf, buf_size - 1);
    printf("Done\n");
    lock_close(file);
    free(buf);
    return 0;
}
