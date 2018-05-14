#include <stdlib.h>
#include <unistd.h>
#include "array.h"
#include <sys/types.h>

ProcessArray* create_array(void) {
    ProcessArray* arr = (ProcessArray*)malloc(sizeof(ProcessArray));
    if (!arr) 
        return NULL;
    arr->arr = (Process*)malloc(STATIC_SIZE * sizeof(Process));
    if (!arr->arr) {
        free(arr);
        return NULL;
    }
    arr->size = STATIC_SIZE;
    arr->len = 0;
    return arr;
}

ProcessArray* arr_push(ProcessArray* arr, Process* element) {
    if (arr->size == arr->len) {
        arr->arr = (Process*)realloc(arr->arr, (arr->size + STATIC_SIZE) * sizeof(Process));
        arr->size += STATIC_SIZE;
    }
    if (!arr->arr) {
        free(arr);
        return NULL;
    }
    arr->arr[arr->len++] = *element;
    return arr;
}

void arr_delete(ProcessArray* arr, int idx) {
    if (idx >= arr->len) {
        return;
    }
    unlink(arr->arr[idx].pid_file);
    free(arr->arr[idx].pid_file);
    int j = 0;
    while (arr->arr[idx].argv[j])
        free(arr->arr[idx].argv[j++]);
    free(arr->arr[idx].argv);
    for (int i = idx; i < arr->len - 1; i++) {
        arr->arr[i] = arr->arr[i+1];
    }
    arr->len--;
}
