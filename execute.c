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

#define FIN      0
#define FOUT     1
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
    int i, nc, status, prevfd, fd;
    int pfds[2];
    pid_t pid;
    char *filename;
    struct sish_command *tmp, *prev;

    status = last_status;
    nc = ncommands(comm);
    fd = -1;

    prevfd = STDIN_FILENO;

    print_command(comm);

    for (i = 0, tmp = comm, prev = NULL; i < nc;
	 i++, prev = tmp, tmp = tmp->next) {
	
	if (pipe(pfds) < 0)
	    err(127, "pipe");

	if ((pid = fork()) < 0)
	    err(127, "fork");

	if (pid == 0) { /* child */
	    if (dup2(prevfd, STDIN_FILENO) != STDIN_FILENO)
		err(127, "dup2 stdin");

	    if (tmp->next != NULL)
		filename = tmp->next->command;

	    switch (tmp->conn) {
	    case PIPE:
		if (dup2(pfds[FOUT], STDOUT_FILENO) != STDOUT_FILENO)
		    err(127, "dup2 stdout");
		/* FALLTHROUGH */
		
	    case OUT:
		close(pfds[FIN]);
		if ((fd = open(filename, WR_FLAGS, DEFAULT_MODE)) == -1)
		    err(127, "open");

		if (dup2(fd, STDOUT_FILENO) != STDOUT_FILENO)
		    err(127, "dup2 stdout");
		break;

	    case IN:
		close(pfds[FOUT]);
		if ((fd = open(filename, RD_FLAGS)) == -1)
		    err(127, "open");

		if (dup2(fd, STDIN_FILENO) != STDIN_FILENO)
		    err(127, "dup2 stdin");
		break;
		
	    case APPEND:
		close(pfds[FIN]);
		if ((fd = open(filename, AP_FLAGS, DEFAULT_MODE)) == -1)
		    err(127, "open");

		if (dup2(fd, STDOUT_FILENO) != STDOUT_FILENO)
		    err(127, "dup2 stdout");
		break;

	    default:
		close(pfds[FIN]);
		break;
	    }

	    if (prev && (prev->conn == IN || prev->conn == OUT
			 || prev->conn == APPEND)) {
		printf("prev->command: %s\n", prev->command);
		exit(EXIT_SUCCESS);
	    }

	    status = execvp(tmp->command, tmp->argv);
	    fprintf(stderr, "exec: %s: %s\n", tmp->command, strerror(errno));
	    exit(127);
	} else { /* parent */
	    
	    if (waitpid(pid, &status, 0) < 0)
		err(127, "waitpid");

	    close(fd);
	    close(pfds[FOUT]);
	    prevfd = pfds[FIN];
	}
    }

    return status;
}
