#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "parse.h"

int
sish_execute(struct sish_command *comm)
{
    int n, p[2], status, i;
    char *buf, **args;
    pid_t pid;

    status = EXIT_FAILURE;

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

	if (waitpid(pid, NULL, 0) < 0)
	    err(EXIT_FAILURE, "waitpid");

	return status;
    } else { /* child */
	close(p[0]); /* close read end */

	args[0] = comm->command;
	for (i = 1; i < comm->argc + 1; i++) {
	    args[i] = comm->argv[i-1];
	}
	args[i] = NULL;

	status = execvp(comm->command, args);
	
	err(EXIT_FAILURE, "execvp");
    }
}
