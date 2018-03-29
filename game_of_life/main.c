#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define ALIVE   '#'
#define DEAD    ' '

unsigned char** map;
int width;
int height;

void print_help(const char* const prog) {
    printf(
        "Usage: %s MAP\n\n"
        "Parameters:\n"
        "  MAP     path to file with map\n\n"
        "You can use map_generator.py to generate fields.\n\n"
        "Map example:\n"
        "  0000#00\n"
        "  00###00\n"
        "  000#000\n\n"
        "(C) defntvdm 2018\n"
    , prog);
}

void read_data(const char* const file_name) {
    int fd = open(file_name, 0, O_RDONLY);
    if (fd < 0) {
        perror("Open file error");
        exit(1);
    }
    struct stat file_stat;
    stat(file_name, &file_stat);
    unsigned char *buffer = (unsigned char*)malloc(file_stat.st_size);
    if (buffer == NULL) {
        perror("Malloc error");
        exit(1);
    }
    int readed = read(fd, buffer, file_stat.st_size);
    if (readed < 0) {
        perror("Read file error");
        exit(1);
    }
    char* new_line = strchr((char*)buffer, '\n');
    width = new_line - (char*)buffer;
    height = file_stat.st_size / (width + 1);
    map = malloc(height*sizeof(char*));
    for (int i = 0; i < height; i++) {
        puts("DEBUG");
        map[i] = mmap(
            NULL,
            width,
            O_RDWR,
            MAP_SHARED | MAP_ANONYMOUS,
            -1,
            0
        );
        if (map[i] < 0) {
            perror("mmap error");
            exit(1);
        }
    }
    for (int i = 0; i < height; i++) {
        memcpy(map[i], buffer, width);
        buffer += width + 1;
    }
    close(fd);
}

void wait_connection(int fd) {
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(addr);
    int sock = accept(fd, (struct sockaddr*)&addr, &addr_size);
    if (sock < 0) {
        perror("Connection error");
        return;
    } else {
        printf("Connection from: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    }
    for (int i = 0; i < height; i++) {
        write(sock, map[i], width);
        write(sock, "\n", 1);
    }
    close(sock);
}

void handle(void) {
    int server = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8000);
    bind(server, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
    listen(server, 5);
    while (1) {
        wait_connection(server);
    }
}

int count_neighbours(int i, int j) {
    int res = 0;
    int coords[16] = {
        i-1, j-1,
        i-1, j,
        i-1, j+1,
        i, j-1,
        i, j+1,
        i+1, j-1,
        i+1, j,
        i+1, j+1
    };
    for (int k = 0; k < 16; k += 2) {
        int x = coords[k];
        int y = coords[k+1];
        if (x > -1 && x < height && y > -1 && y < width) {
            if (map[x][y] == ALIVE)
                res++;
        }
    }
    return res;
}

void recalculate_map(void) {
    unsigned char** tmp = (unsigned char**)malloc(height*sizeof(unsigned char*));
    for (int i = 0; i < height; i++)
        tmp[i] = (unsigned char*)malloc(width); 
    while (1) {
        for (int i = 0; i < height; i++)
            memcpy(tmp[i], map[i], width);
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                int neighs = count_neighbours(i, j);
                if (map[i][j] == DEAD) {
                    if (neighs == 3) {
                        tmp[i][j] = ALIVE;
                    }
                } else {
                    if (neighs != 2 && neighs != 3) {
                        tmp[i][j] = DEAD;
                    }
                }
            }
        }
        for (int i = 0; i < height; i++)
            memcpy(map[i], tmp[i], width);
        usleep(950000);
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        print_help(argv[0]);
        return 0;
    }
    read_data(argv[1]);
    int pid = fork();
    if (pid < 0) {
        perror("Fork error");
        return 1;
    } else if (pid > 0) {
        puts("Parent good");
        handle();
    } else {
        puts("Child good");
        recalculate_map();
    }
}
