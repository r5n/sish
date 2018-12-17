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
#define AP_FLAGS O_WRONLY | O_CREAT | O_APPEND
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
sish_execute(struct sish_command *cmd)
{
    int i, nc, status, prevfd, fd, ex;
    int p[2];
    pid_t pid;
    char *filename;
    struct sish_command *curr, *prev;

    status = last_status;
    nc = ncommands(cmd);
    fd = -1;
    ex = 1;

    prevfd = STDIN_FILENO;

    for (i = 0, curr = cmd, prev = NULL; i < nc;
	 i++, prev = curr, curr = curr->next) {

	if (pipe(p) < 0)
	    err(127, "pipe");

	if (prev)
	    switch (prev->conn) {
	    case IN: case OUT: case APPEND:
		ex = 0;
		break;
	    default:
		ex = 1;
		break;
	    }

	if ((pid = fork()) < 0)
	    err(127, "fork");

	if (pid == 0) { /* child */

	    if (dup2(prevfd, STDIN_FILENO) != STDIN_FILENO)
		err(127, "dup2 stdin");

	    switch (curr->conn) {
	    case PIPE:		
		if (dup2(p[FOUT], STDOUT_FILENO) != STDOUT_FILENO)
		    err(127, "dup2 stdout");

		break;

	    case IN:
		filename = curr->next->command;
		if ((fd = open(filename, RD_FLAGS)) == -1)
		    err(127, "open");

		if (dup2(fd, STDIN_FILENO) != STDIN_FILENO)
		    err(127, "dup2 stdin");

		break;

	    case OUT:
		filename = curr->next->command;
		if ((fd = open(filename, WR_FLAGS, DEFAULT_MODE)) == -1)
		    err(127, "open");

		(void)close(p[FOUT]); /* close write end -- set to fd */
		
		if (dup2(fd, STDOUT_FILENO) != STDOUT_FILENO)
		    err(127, "dup2 stdout");

		break;

	    case APPEND:
		filename = curr->next->command;
		if ((fd = open(filename, AP_FLAGS, DEFAULT_MODE)) == -1)
		    err(127, "open");

		(void)close(p[FOUT]); /* close write end -- set to fd */

		if (dup2(fd, STDOUT_FILENO) != STDOUT_FILENO)
		    err(127, "dup2 stdout");

		break;

	    default:
		break;
	    }

	    (void)close(p[FIN]); /* close read end */

	    if (!ex) exit(0); /* curr->command refers to a file */
	    
	    status = execvp(curr->command, curr->argv);
	    fprintf(stderr, "%s: %s\n", curr->command, strerror(errno));
	    exit(127);
	} else { /* parent */

	    if (waitpid(pid, &status, 0) < 0)
		err(127, "waitpid");

	    (void)close(p[FOUT]); /* close write end */
	    prevfd = p[FIN];      /* save write end of child */

	}
    }

    (void)close(prevfd);
    return status;
}
