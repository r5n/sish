#include <sys/types.h>
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

#define IN       0
#define OUT      1
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
    int i, nc, status, prevfd;
    int fd[2];
    pid_t pid;
    struct sish_command *tmp;

    status = last_status;
    nc = ncommands(comm);

    prevfd = STDIN_FILENO;

    for (i = 0, tmp = comm; i < nc; i++, tmp = tmp->next) {
	if (pipe(fd) < 0)
	    err(127, "pipe");

	if ((pid = fork()) < 0)
	    err(127, "fork");

	if (pid == 0) { /* child */
	    if (dup2(prevfd, STDIN_FILENO) != STDIN_FILENO)
		err(127, "dup2 stdin");

	    if (tmp->conn == PIPE)
		if (dup2(fd[OUT], STDOUT_FILENO) != STDOUT_FILENO)
		    err(127, "dup2 stdout");

	    close(fd[IN]);
	    status = execvp(tmp->command, tmp->argv);
	    err(127, "execvp");
	} else { /* parent */
	    
	    if (waitpid(pid, &status, 0) < 0)
		err(127, "waitpid");

	    close(fd[OUT]);
	    prevfd = fd[IN];
	}
    }

    return status;
}
