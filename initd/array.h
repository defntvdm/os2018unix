#include <unistd.h>
#include <sys/types.h>

#ifndef __PROCARRAY
#define __PROCARRAY

typedef struct {
    pid_t    pid;
    char**   argv;
    char*    pid_file;
    int      error_count;
    char     respawn;
} Process;

typedef struct {
    Process*   arr;
    int        size;
    int        len;
} ProcessArray;

#endif

#ifndef __ARRAYFUNC
#define __ARRAYFUNC

#define STATIC_SIZE 10
ProcessArray* create_array(void);
ProcessArray* arr_push(ProcessArray* arr, Process* element);
void arr_delete(ProcessArray* arr, int i);
#endif
