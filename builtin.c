#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parse.h"

int match(const char *, const char *);
void builtin_cd(const char *);
void builtin_echo(const char *);
void builtin_exit(void);

void
builtin(struct sish_command *comm)
{
    if (match(comm->command, "exit"))
	builtin_exit();

    if (match(comm->command, "cd")) {
	if (comm->argc > 1) {
	    fprintf(stderr, "usage: cd [dir]");
	    return;
	}
	
	if (comm->argc == 0)
	    return;
	
	builtin_cd((comm->argv)[0]);
	return;
    }

    if (match(comm->command, "echo"))
	builtin_echo((comm->argv)[0]);

    return;
}

void
builtin_cd(const char *path)
{
    /* TODO: call realpath */
    if ((chdir(path)) == -1) {
	fprintf(stderr, "cd: %s\n", strerror(errno));
    }
}

void
builtin_echo(const char *str)
{
    /* TODO: support for printing out exit status of last command */
    if (match(str, "$$")) {
	printf("%ld\n", (long)getpid());
	return;
    }
}

void
builtin_exit(void)
{
    exit(EXIT_SUCCESS);
}

int
match(const char *src, const char *dest)
{
    int slen, dlen;

    if (src == NULL)
	return 0;

    slen = strlen(src);
    dlen = strlen(dest);

    if (slen != dlen)
	return 0;

    if ((strncmp(src, dest, slen)) == 0)
	return 1;

    return 0;
}

