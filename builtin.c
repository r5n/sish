#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parse.h"
#include "util.h"

#define HOME "HOME"

void builtin_cd(const char *, int, int *);
void builtin_echo(char **, int, int, int*);
void builtin_exit(int);

int
sish_builtin(struct sish_command *comm, int trace, int* last_status)
{
    if (match(comm->command, "exit"))
        builtin_exit(trace);

    if (match(comm->command, "cd")) {
        if (comm->argc > 1) {
            fprintf(stderr, "usage: cd [dir]");
            return 1;
        }

        builtin_cd((comm->argv)[1], trace, last_status);
        return 1;
    }

    if (match(comm->command, "echo")) {
        builtin_echo(comm->argv, comm->argc + 1, trace, last_status);
        return 1;
    }
    return 0;
}

void
builtin_cd(const char *path, int trace, int *last_status)
{
    const char *dir;

    dir = path;
    if (trace == 1)
        printf("%s %s\n", "+ cd", (dir ? dir : ""));

    if (!dir || match(dir, "\n")) {
        if ((dir = getenv(HOME)) == NULL) {
            *last_status = 127;
            return;
        }
    }
    if ((chdir(dir)) == -1) {
        *last_status = 127;
        fprintf(stderr, "cd: %s: %s\n", dir, strerror(errno));
    }

    *last_status = 0;
}

void
builtin_echo(char **str, int len, int trace, int *last_status)
{
    int i;

    if (trace == 1) {
        printf("%s", "+ echo");
        for (i = 1; i < len; i++)
            printf(" %s", str[i]);
        printf("\n");
    }

    for (i = 1; i < len; i++) {
        if (match(str[i], "$$"))
            printf("%ld ", (long)getpid());
        else if (match(str[i], "$?"))
            printf("%d ", *last_status);
        else
            printf("%s ", str[i]);
    }

    printf("\n");
    *last_status = 0;
}

void
builtin_exit(int trace)
{
    if (trace == 1)
        printf("%s\n", "+ exit");

    exit(EXIT_SUCCESS);
}
