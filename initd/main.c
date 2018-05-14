#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include "array.h"

#define RAND_LEN 10
#define ALPH_LEN 62

ProcessArray* childs;
const char* delims = " ";
char* alph = "0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM";
char* config_file = "/etc/myinit.conf";
char* old;

void start_watching(void);

void sig_hup_handler(int sig) {
    signal(SIGHUP, sig_hup_handler);
    syslog(LOG_INFO, "Got HUP signal, killing childs");
    for (int i = 0; i < childs->len; i++) {
        kill(childs->arr[i].pid, SIGKILL);
        free(childs->arr[i].pid_file);
        int j = 0;
        while (childs->arr[i].argv[j])
            free(childs->arr[i].argv[j++]);
        free(childs->arr[i].argv);
    }
    free(childs->arr);
    free(childs);
    start_watching();
}

void sig_term_handler(int sig) {
    closelog();
    _exit(0);
}

void free_argv(char** argv, int size) {
    for (int i = 0; i < size; i++) {
        free(argv[i]);
    }
    free(argv);
}

char* get_random_filename(size_t len) {
    int num;
    char* result = malloc(len + 10); // 10 - длина "/tmp/.pid\0"
    if (!result)
        return NULL;
    strcpy(result, "/tmp/");
    strcpy(&result[5 + len], ".pid");
    result[10 + len] = 0;
    for (size_t i = 5; i < 5 + len; i++) {
        num = random();
        result[i] = alph[num % ALPH_LEN];
    }
    return result;
}

int write_to_file(char* pid_file, pid_t pid) {
    FILE* f = fopen(pid_file, "w");
    if (!f) {
        return 1;
    }
    if (!fprintf(f, "%d", pid)) {
        free(f);
        return 1;
    }
    fflush(f);
    free(f);
    return 0;
}

char* tokenize(char* start, char delim) {
    char* result;
    old++;
    start = old;
    while (*old != delim && *old != '\n' && *old != '\0') old++;
    if (*old == '"') {
        result = (char*)malloc(old - start);
        if (!result)
            syslog(LOG_ERR, "%s", strerror(errno));                                        
        else {
            if (old - start == 0)
                return 0;
            memcpy(result, start, old - start);
            result[old-start] = '\0';
            old++;
        }
    }
    else
        result = NULL;
    return result;
}

char* get_token(const char* const line) {
    char *start_token, *token;
    if (line)
        old = (char*)line;
    if (*old == '\0' || *old == '\n')
        return NULL;
    while (*old == ' ') old++;
    switch (*old) {
        case '"':
            return tokenize(start_token, '"');
        case '\'':
            return tokenize(start_token, '\'');
        default:
            start_token = old;
            while (*old != ' ' && *old != '\n' && *old != '\0') old++;
            token = (char*)malloc(old - start_token);
            if (!token)
                syslog(LOG_ERR, "%s", strerror(errno));
            else {
                if (old - start_token == 0)
                    return 0;
                memcpy(token, start_token, old - start_token);
                token[old-start_token] = '\0';                
            }
            return token;
    }
}

int parse_line(const char* line, Process* proc) {
    char* token;
    char** ptr;
    int size = 0;
    proc->pid = 0;
    proc->argv = NULL;
    token = get_token(line);
    if (!token)
        return 2;
    while (1) {
        if ((ptr = realloc(proc->argv, sizeof(char**) * (++size))) == NULL) {
            syslog(LOG_ERR, "%s", strerror(errno));
            free_argv(proc->argv, size);
            return 1;
        }
        proc->argv = ptr;
        proc->argv[size - 1] = token;
        if (!token)
            break;
        token = get_token(NULL);
    }
    if (size < 3) {
        syslog(LOG_WARNING, "Skip line `%s` last parameter must be %s or %s", line, "respawn", "wait");
        free_argv(proc->argv, size);
        return 2;
    }
    if (!strcmp(proc->argv[size-2], "respawn")) {
        proc->respawn = 1;    
    } else if (!strcmp(proc->argv[size-2], "wait")) {
        proc->respawn = 0;
    } else {
        syslog(LOG_WARNING, "Skip line `%s` last parameter must be %s or %s", line, "respawn", "wait");
        free_argv(proc->argv, size);
        return 2;
    }
    free(proc->argv[size-2]);
    proc->argv[size-2] = NULL;
    proc->argv = realloc(proc->argv, --size * sizeof(char**));
    proc->pid_file = get_random_filename(RAND_LEN);
    if (!proc->pid_file)
        syslog(LOG_WARNING, "Couldn't ");
    return 0;
}

void start_child(int i) {
    pid_t cpid = fork();
    switch (cpid) {
        case -1:
            syslog(LOG_WARNING, "Can't fork myself");
            break;
        case 0:
            execvp(childs->arr[i].argv[0], childs->arr[i].argv);
            _exit('Z'); // ord('Z') = 0x5a
            break;
        default:
            childs->arr[i].pid = cpid;
            if (write_to_file(childs->arr[i].pid_file, cpid))
                syslog(LOG_WARNING, "Couldn't write pid to file %s", childs->arr[i].pid_file);
            break;
    }
}

void start_watching() {
    int cpid;
    childs = create_array();
    FILE* config = fopen(config_file, "r");
    if (config == NULL) {
        syslog(LOG_ERR, "Couldn't open config file %s. Try to use abs path.", config_file);
        closelog();
        return;
    }
    syslog(LOG_INFO, "Start parsing config file (%s)", config_file);
    char* line;
    ssize_t got;
    Process proc;
    proc.error_count = 0;
    childs = create_array();
    while (1) {
        line = NULL;
        got = getline(&line, &(size_t){0}, config);
        if (got == -1)
            break;
        line[got-1] = '\0';
        switch (parse_line(line, &proc)) {
            case 0:
                arr_push(childs, &proc);
                break;
            case 1:
                syslog(LOG_WARNING, "Skip line `%s`, invalid format", line);
                break;
            default:
                break;
        }
        free(line);
    }
    fclose(config);
    syslog(LOG_INFO, "Starting child processes...");
    for (int i = 0; i < childs->len; i++) {
        start_child(i);
    }
    syslog(LOG_INFO, "Monitoring...");
    int status;
    while (childs->len) {
        cpid = wait(&status);
        for (int i = 0; i < childs->len; i++) {
            if (childs->arr[i].pid == cpid) {
                syslog(LOG_NOTICE, "%s [%d] exited with status %d", childs->arr[i].argv[0], cpid, status);
                if (status == 0x5a00) {
                    if (childs->arr[i].error_count++ == 50) {
                        syslog(LOG_ERR, "%s failed 50 times, ignore it.", childs->arr[i].argv[0]);
                        arr_delete(childs, i);
                    }
                    start_child(i);
                } else {
                    if (childs->arr[i].respawn)
                        start_child(i);
                    else
                        arr_delete(childs, i);
                }
                break;
            }
        }
    }
}

int main(int argc, char** argv) {
    if (argc >= 2)
        config_file = argv[1];
    int cpid = fork();
    struct rlimit rlim;
    if (cpid < 0) {
        perror("Damn");
        return 1;
    } else if (cpid > 0) {
        return 0;
    }
    printf("%d\n", getpid());
    srand(time(NULL));
    setsid();
    chdir("/");
    signal(SIGTERM, sig_term_handler);
    signal(SIGHUP, sig_hup_handler);
    getrlimit(RLIMIT_NOFILE, &rlim);
    for (int fd = 0; fd < rlim.rlim_max; fd++)
        close(fd);
    openlog("init", LOG_CONS & ~LOG_PID, LOG_DAEMON);
    start_watching();
}
