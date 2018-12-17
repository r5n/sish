#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parse.h"
#include "util.h"

extern int last_status;

void builtin_cd(const char *, int);
void builtin_echo(const char *, int);
void builtin_exit(int);

int
sish_builtin(struct sish_command *comm, int trace)
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
	
	builtin_cd((comm->argv)[1], trace);
	return 1;
    }

    if (match(comm->command, "echo")) {
	builtin_echo((comm->argv)[1], trace);
	return 1;
    }
    return 0;
}

void
builtin_cd(const char *path, int trace)
{
    if (trace == 1)
	printf("%s: %s\n", "+ cd", path);
    
    if ((chdir(path)) == -1) {
	fprintf(stderr, "cd: %s\n", strerror(errno));
    }
}

void
builtin_echo(const char *str, int trace)
{
    if (trace == 1)
	printf("%s %s\n", "+ echo", str);
    
    if (match(str, "$$")) {
	printf("%ld", (long)getpid());
    } else if (match(str, "$?")) {
	printf("%d", last_status);
    } else {
	printf("%s", str);
    }
    printf("\n");
}

void
builtin_exit(int trace)
{
    if (trace == 1)
	printf("%s\n", "+ exit");
    
    exit(EXIT_SUCCESS);
}
