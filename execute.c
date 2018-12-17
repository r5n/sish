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
    int nc, status, prevfd, fdout, fdin, i;
    int p[2];
    pid_t pid;
    struct sish_command *curr;

    status = last_status;
    nc = ncommands(cmd);
    prevfd = STDIN_FILENO;
    curr = cmd;
    fdout = fdin = -1;

    print_command(cmd);

    for (i = 0; i < nc; curr = curr->next, i++) {
	if (pipe(p) < 0)
	    err(127, "pipe");

	if ((pid = fork()) < 0)
	    err(127, "fork");

	if (pid == 0) { /* child */
	    if (dup2(prevfd, STDIN_FILENO) != STDIN_FILENO)
	    	err(127, "dup2 stdin");

	    if (curr->stdin) {
		if ((fdin = open(curr->stdin, RD_FLAGS)) == -1)
		    err(127, "open");

		if (dup2(fdin, STDIN_FILENO) != STDIN_FILENO)
		    err(127, "dup2 stdin");
	    }
	    if (curr->stdout) {
		if ((fdout = open(curr->stdout,
				  (curr->append == 1 ? AP_FLAGS : WR_FLAGS),
				  DEFAULT_MODE)) == -1)
		    err(127, "open");

		if (dup2(fdout, STDOUT_FILENO) != STDOUT_FILENO)
		    err(127, "dup2 stdout");
	    }
	    if (curr->conn == PIPE) {
		if (dup2(p[FOUT], STDOUT_FILENO) != STDOUT_FILENO)
		    err(127, "dup2 stdout");
	    }

	    (void)close(p[FIN]); /* close read end */

	    printf("command: %s\n", curr->command);
	    status = execvp(curr->command, curr->argv);
	    fprintf(stderr, "%s: %s\n", curr->command, strerror(errno));
	    exit(127);
	} else { /* parent */
	    if (waitpid(pid, &status, 0) < 0)
		err(127, "waitpid");

	    (void)close(p[FOUT]);
	    prevfd = p[FIN];
	}
    }

    (void)close(prevfd);
    if (fdin != -1)
	(void)close(fdin);
    if (fdout != -1)
	(void)close(fdout);

    return status;
}
