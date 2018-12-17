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
    int nc, status, prevfd, fd, i;
    int p[2];
    pid_t pid;
    char *filename;
    struct sish_command *curr, *prev;

    status = last_status;
    nc = ncommands(cmd);
    prevfd = fd = STDIN_FILENO;
    curr = cmd;

    for (i = 0; i < nc; i++) {
	prev = curr;
	
	if (pipe(p) < 0)
	    err(127, "pipe");

	if ((pid = fork()) < 0)
	    err(127, "fork");

	if (pid == 0) { /* child */

	    if (dup2(prevfd, STDIN_FILENO) != STDIN_FILENO)
		err(127, "dup2 stdin");

	    if (curr->conn == PIPE) {
		if (dup2(p[FOUT], STDOUT_FILENO) != STDOUT_FILENO)
		    err(127, "dup2 stdout");
	    } else {
		for (; curr; i++, curr = curr->next) {
		    if (curr->conn == IN) {
			filename = curr->next->command;
			if ((fd = open(filename, RD_FLAGS)) == -1)
			    err(127, "open");
			if (dup2(fd, STDIN_FILENO) != STDIN_FILENO)
			    err(127, "dup2 stdin");
		    } else if (curr->conn == OUT) {
			filename = curr->next->command;
			if ((fd = open(filename, WR_FLAGS, DEFAULT_MODE)) == -1)
			    err(127, "open");
			if (dup2(fd, STDOUT_FILENO) != STDOUT_FILENO)
			    err(127, "dup2 stdout");
		    } else if (curr->conn == APPEND) {
			filename = curr->next->command;
			if ((fd = open(filename, AP_FLAGS, DEFAULT_MODE)) == -1)
			    err(127, "open");
			if (dup2(fd, STDOUT_FILENO) != STDOUT_FILENO)
			    err(127, "dup2 stdout");
		    }
		    else
			break;
		}
	    }

	    (void)close(p[FIN]); /* close read end */

	    printf("command: %s\n", prev->command);
	    status = execvp(prev->command, prev->argv);
	    fprintf(stderr, "exec: %s: %s\n", prev->command, strerror(errno));
	    exit(127);
	} else {
	    if (waitpid(pid, &status, 0) < 0)
		err(127, "waitpid");

	    (void)close(p[FOUT]);
	    prevfd = p[FIN];
	}
    }

    (void)close(prevfd);
    return status;
}
