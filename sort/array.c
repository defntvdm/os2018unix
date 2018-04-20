#include <stdlib.h>
#include "array.h"

Array* create_array(void) {
    Array* arr = (Array*)malloc(sizeof(Array));
    if (!arr) 
        return NULL;
    arr->arr = (long long*)malloc(STATIC_SIZE * sizeof(long long));
    if (!arr->arr) {
        free(arr);
        return NULL;
    }
    arr->size = STATIC_SIZE;
    arr->len = 0;
    return arr;
}

Array* arr_push(Array* arr, long long element) {
    if (arr->size == arr->len) {
        arr->arr = (long long*)realloc(arr->arr, (arr->size + STATIC_SIZE) * sizeof(long long));
        arr->size += STATIC_SIZE;
    }
    if (!arr->arr) {
        return NULL;
    }
    arr->arr[arr->len++] = element;
    return arr;
}

int qsort_comparator(const void* a, const void* b) {
    if (*(long long*)a > *(long long*)b)
        return 1;
    else if (*(long long*)a < *(long long*)b)
        return -1;
    else
        return 0;
}
