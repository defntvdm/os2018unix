typedef struct {
    long long* arr;
    int        size;
    int        len;
} Array;

#define STATIC_SIZE 100

Array* create_array(void);
Array* arr_push(Array* arr, long long element);
int qsort_comparator(const void* a, const void* b);
