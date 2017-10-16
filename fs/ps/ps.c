#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#ifdef DEBUG
#define pr_debug(...) eprintf(__VA_ARGS__)
#else
#define pr_debug(...) do {} while (0)
#endif

#define MAX_PATH_SIZE 64
#define MAX_STATUS_SIZE 2048

#define PAGER "less"

struct pid_stat_t {
    char *status;
    pid_t pid;
    char *name;
    char *state;
    char *vmsize;
    char *threads;
};

char *get_field_tail(const char *status, const char *field)
{
    char *tail = (char *)memmem(status, MAX_STATUS_SIZE, field, strlen(field));
    if (!tail)
        return NULL;
    tail = strtok(tail, " \t"); 
    tail = strtok(NULL, " \t"); 
    return tail;
}

int pid_stat_print(FILE *file, struct pid_stat_t *ps)
{
    return fprintf(file, "%5d %24s %8s %12s %7s\n", ps->pid, ps->name, ps->state,
            ps->vmsize, ps->threads); 
}

int pid_stat_create(int pid, struct pid_stat_t *ps)
{
    int err = 0;
    char path[MAX_PATH_SIZE] = {};
    
    snprintf(path, MAX_PATH_SIZE, "/proc/%d/status", pid);

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        pr_debug("open: %s\n", strerror(errno));
        return -errno;
    }

    ps->status = malloc(MAX_STATUS_SIZE * sizeof(char));
    ps->status[MAX_STATUS_SIZE - 1] = '\0';
    err = read(fd, ps->status, MAX_STATUS_SIZE - 1);
    if (err < 0) {
        pr_debug("read: %s\n", strerror(errno));
        err = -errno;
        goto out_close;
    }
    
    err = 0;
    
    ps->pid = pid;
    ps->name = get_field_tail(ps->status, "Name");
    ps->state = get_field_tail(ps->status, "State");
    ps->vmsize = get_field_tail(ps->status, "VmSize");
    ps->threads = get_field_tail(ps->status, "Threads");

    for (size_t i = 0; i < MAX_STATUS_SIZE; ++i) {
        if (ps->status[i] == '\n')
            ps->status[i] = '\0'; 
    }

out_close:
    close(fd);
    return err;
}

int print_header(FILE *file)
{
    return fprintf(file, "%5s %24s %8s %12s %7s\n", "PID", "COMM", "STATE", "VMSIZE", "THREADS");
}

void pid_stat_destroy(struct pid_stat_t *ps)
{
    free(ps->status);
}

int main(int argc, char *argv[])
{
    int err = 0;
    const char usage[] = "usage: ps [pid]";
  
    if (argc == 1) {
        struct pid_stat_t ps;
        FILE *pager = popen(PAGER, "w");
        if (!pager) {
            eprintf("Can't run pager '%s'\n", PAGER);
            return -1;
        }

        print_header(pager);
        for (int pid = 1; pid < INT16_MAX; ++pid) {
            err = pid_stat_create(pid, &ps);
            if (!err) {
                pid_stat_print(pager, &ps);
                pid_stat_destroy(&ps);
            }
            err = 0;
        }

        pclose(pager);
    } else if (argc == 2) {
        int pid = atoi(argv[1]);
        struct pid_stat_t ps;
        
        err = pid_stat_create(pid, &ps);
        if (err < 0) {
            eprintf("pid_stat_create: %s\n", strerror(-err));
            return -1;
        }
        
        print_header(stdout);
        pid_stat_print(stdout, &ps);
        pid_stat_destroy(&ps);
    } else {
        eprintf("%s\n", usage);
    }

    return err;
}
