#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "array.h"

#define BUFF_SIZE 1024

unsigned char* buffer;

int parse_file(const char* filename, Array* arr) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Open file error");
        fprintf(stderr, "Пропускаем файл %s\n", filename);
        return 0;
    }
    int c = 0;
    char skip = 0, scan = 0;
    int idx = 0, got = 0;
    long long number = 0;
    while (1) {
        c = read(fd, buffer + idx, BUFF_SIZE - idx);
        c = c <= 0 ? idx : c + idx;
        if (!c)
            break;
        idx = 0;
        if (skip) {
            while (idx < c && buffer[idx] >= '0' && buffer[idx] <= '9')
                idx++;
            if (idx != c) {
                skip = 0;
                memcpy(buffer, buffer + idx, c - idx);
                idx = c - idx;
            } else
                idx = 0;
            continue;
        }
        if (!scan) {
            while (idx < c && (buffer[idx] < '0' || buffer[idx] > '9') && buffer[idx] != '-')
                idx++;
            if (idx != c) {
                scan = 1;
                memcpy(buffer, buffer + idx, c - idx);
                idx = c - idx;
            } else
                idx = 0;
            continue;
        }
        errno = 0; // почему-то оно само на success при удачном sscanf не выставляется после ошибок
        got = sscanf((char*)buffer, "%lld", &number);
        if (!got) {
            // скорее всего я просто увидел одинокий -
            buffer[0] = 'A';
            scan = 0;
            idx = c; // не нужно перезаписывать буффер, нужно искать следующее вхождение
            continue;
        }
        if (errno) {
            perror("Skip big number");
        } else
            if(!arr_push(arr, number)) {
                return 1;
            }
        if (buffer[0] == '-')
            buffer[0] = '1';
        idx = c;
        scan = 0;
        skip = 1;
        continue;
    }
    close(fd);
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        fputs("Должно быть не менее 3 параметров\n", stderr);
        return 1;
    }
    FILE* res_file = fopen(argv[argc-1], "w");
    if (res_file == NULL) {
        fputs("Невозможно открыть результирующий файл\n", stderr);
        return 1;
    }
    buffer = (unsigned char*)malloc(BUFF_SIZE+1);
    buffer[BUFF_SIZE] = 0x00;
    if (buffer == NULL) {
        perror("Can't malloc memory for buffer");
        return 1;
    }
    Array* arr = create_array();
    if (arr == NULL) {
        perror("Can't malloc memory for array of numbers");
        return 1;
    }
    for (int i = 1; i < argc - 1; i++) {
        if (parse_file(argv[i], arr)) {
            perror("Rellocation error");
            fprintf(stderr, "Не смогли перевыделить память, прощайте(((");
            return 1;
        }
    }
    fputs("Начинаем сортировку...\n", stderr);
    qsort(arr->arr, arr->len, sizeof(long long), qsort_comparator);
    char number[25];
    int len;
    fputs("Дампим массив в файл...\n", stderr);
    for (int i = 0; i < arr->len; i++) {
        fprintf(res_file, "%lld\n", arr->arr[i]);
    }
}
