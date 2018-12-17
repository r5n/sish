#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parse.h"
#include "util.h"

void builtin_cd(const char *, int, int *);
void builtin_echo(const char *, int, int*);
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
	
	if (comm->argc == 0)
	    return 1;
	
	builtin_cd((comm->argv)[1], trace, last_status);
	return 1;
    }

    if (match(comm->command, "echo")) {
	builtin_echo((comm->argv)[1], trace, last_status);
	return 1;
    }
    return 0;
}

void
builtin_cd(const char *path, int trace, int *last_status)
{
    if (trace == 1)
	printf("%s: %s\n", "+ cd", path);
    
    if ((chdir(path)) == -1) {
	*last_status = 127;
	fprintf(stderr, "cd: %s\n", strerror(errno));
    }

    *last_status = 0;
}

void
builtin_echo(const char *str, int trace, int *last_status)
{
    if (trace == 1)
	printf("%s %s\n", "+ echo", str);
    
    if (match(str, "$$")) {
	printf("%ld", (long)getpid());
    } else if (match(str, "$?")) {
	printf("%d", *last_status);
    } else {
	printf("%s", str);
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
