#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parse.h"

#define RD_FLAGS O_RDONLY
#define WR_FLAGS O_WRONLY | O_TRUNC | O_CREAT
#define AP_FLAGS O_WRONLY | O_APPEND
#define DEFAULT_MODE 0644

void
setup_redirection(struct sish_command *comm, int *fd)
{
    char *file;

    if (comm->next == NULL)
	return;

    if ((file = comm->next->command) == NULL)
	return;

    switch(comm->conn) {
    case IN:
	if ((*fd = open(file, RD_FLAGS)) == -1)
	    err(EXIT_FAILURE, "open");

	if (dup2(*fd, STDIN_FILENO) != STDIN_FILENO)
	    err(1, "dup2 to stdin");

	break;
    case OUT:
	if ((*fd = open(file, WR_FLAGS, DEFAULT_MODE)) == -1)
	    err(EXIT_FAILURE, "open");

	if (dup2(*fd, STDOUT_FILENO) != STDOUT_FILENO)
	    err(1, "dup2 to stdout");

	break;
    case APPEND:
	printf("append case!\n");
	if ((*fd = open(file, AP_FLAGS, DEFAULT_MODE)) == -1)
	    err(EXIT_FAILURE, "open");

	if (dup2(*fd, STDOUT_FILENO) != STDOUT_FILENO)
	    err(1, "dup2 to stdout");

	break;
    default:
	return;
    }
}

int
sish_execute(struct sish_command *comm)
{
    int n, p[2], status, i;
    char *buf, **args;
    pid_t pid;

    status = 0;

    if ((buf = malloc(sizeof *buf * BUFSIZ)) == NULL)
	err(1, "malloc");

    if ((args = malloc(sizeof *args * (comm->argc + 1))) == NULL)
	err(1, "malloc");
    
    if (pipe(p) < 0)
	err(EXIT_FAILURE, "pipe");

    if ((pid = fork()) < 0)
	err(EXIT_FAILURE, "fork");

    else if (pid > 0) { /* parent */
	close(p[1]); /* close write end */

	while ((n = read(p[0], buf, BUFSIZ)) > 0) {
	    if (write(STDOUT_FILENO, buf, n) != n)
		err(EXIT_FAILURE, "write");
	}

	free(buf);
	free(args);
	close(p[0]);

	if (waitpid(pid, &status, 0) < 0)
	    err(EXIT_FAILURE, "waitpid");

	return status;
    } else { /* child */
	close(p[0]); /* close read end */

	args[0] = comm->command;
	for (i = 1; i < comm->argc + 1; i++) {
	    args[i] = comm->argv[i-1];
	}
	args[i] = NULL;

	setup_redirection(comm, &(p[1]));

	status = execvp(comm->command, args);
	
	fprintf(stderr, "%s: command not found\n", comm->command);
	status = 127;
	exit(EXIT_FAILURE);
    }
}
