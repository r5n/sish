#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parse.h"

#define RD_FLAGS O_RDONLY
#define WR_FLAGS O_WRONLY | O_TRUNC | O_CREAT
#define AP_FLAGS O_WRONLY | O_APPEND
#define DEFAULT_MODE 0644

extern int last_status;


int
ncommands(struct sish_command *comm)
{
    int n;
    struct sish_command *tmp;
    tmp = comm;
    for (n = 0; tmp; tmp = tmp->next, n++)
	;
    return n;
}

int
sish_execute(struct sish_command *comm)
{
    int i, nc, rd, status;
    int **fds;
    char *buf;
    pid_t *pids;
    struct sish_command *tmp, *prev;
    
    nc = ncommands(comm);

    if ((pids = malloc(sizeof *pids * nc)) == NULL)
	err(127, "malloc");

    if ((fds = malloc(sizeof *fds * nc)) == NULL)
	err(127, "malloc");

    if ((buf = malloc(BUFSIZ)) == NULL)
	err(127, "malloc");

    for (i = 0; i < nc; i++)
	if ((fds[i] = malloc(sizeof *fds[i] * 2)) == NULL)
	    err(127, "malloc");
    
    for (i = 0, tmp = comm, prev = NULL; i < nc;
	 prev = tmp, tmp = tmp->next, i++) {

	if ((pids[i] = fork()) < 0)
	    err(127, "fork");

	if (pids[i] == 0) { /* child */

	    if (prev && prev->conn != PIPE)
		exit(EXIT_SUCCESS);

	    status = execvp(tmp->command, tmp->argv);
	    err(127, "execvp");
	} else { /* parent */
	    if (waitpid(pids[i], &status, 0) < 0)
		err(127, "waitpid");
	}
	
    }

    while ((rd = read(fds[nc-1][0], buf, BUFSIZ)) > 0)
	if (write(STDOUT_FILENO, buf, rd) != rd)
	    err(EXIT_FAILURE, "write");

    for (i = 0; i < nc; i++)
	free(fds[i]);
    free(fds);
    free(pids);
    free(buf);

    return status;
}
