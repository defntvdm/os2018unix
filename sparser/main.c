#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFF_SIZE 1024

unsigned char buffer[BUFF_SIZE]; // Буфер на 1Кб читаем в него и обрабатываем данные

// Небольшая справочка
void help(const char* const cmd) {
    printf(
        "Usage: %s [outfile]\n\n"
        "Parameters:\n"
        "  outfile        Output file\n\n"
        "(C) defntvdm 2018\n"
    , cmd);
}

int main(int argc, char **argv) {
    int r; // количество байт в буфере
    int w; // количество байт для записи в файл
    int s; // количество байт для пропуска (seek)
    unsigned char* ptr; // указатель на текущий обрабатываемый байт
    unsigned char* wptr; // указатель на байт, с которго будем записывать
    int fd = 1; // Если выходной файл не передан, то пишем в stdout
    if (argc > 2) {
        puts("Too many arguments");
        help(argv[0]);
        return 1;
    } else  if (argc == 2) {
        fd = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (fd < 0) {
            perror("Can't open file");
            return 1;
        }
    }
    r = read(0, buffer, BUFF_SIZE);
    while (r) {
        ptr = buffer;
        wptr = buffer;
        w = 0;
        s = 0;
        while (r) {
            if (*ptr) { 
                // Байт не нулевой
                if (s) {
                    // до этого были нулевые
                    wptr = ptr;
                    lseek(fd, s, SEEK_CUR);
                    s = 0;
                }
                w++;
            } else {
                // Байт нулевой
                if (w) {
                    // до этого были ненулевые
                    write(fd, wptr, w);
                    w = 0;
                }
                s++;
            }
            r--;
            ptr++;
        }
        if (w) {
            // Вдруг осталось чего записать
            write(fd, wptr, w);
        } else if (s) {
            // Вдруг осталось чего пропустить
            lseek(fd, s, SEEK_CUR);
        }
        r = read(0, buffer, BUFF_SIZE);
    }
    close(fd);
}
