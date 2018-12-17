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
sish_execute(struct sish_command *cmd, int trace)
{
    int nc, status, prevfd, fdout, fdin, i, j;
    int p[2];
    pid_t pid;
    sigset_t nmask, omask;
    struct sish_command *curr;
    struct sigaction quitsa, sa;

    status = last_status;
    nc = ncommands(cmd);
    prevfd = STDIN_FILENO;
    fdout = fdin = -1;

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGQUIT, &sa, &quitsa) == -1)
	err(127, "sigaction");

    sigemptyset(&nmask);
    sigaddset(&nmask, SIGCHLD);
    if (sigprocmask(SIG_BLOCK, &nmask, &omask) == -1) {
	sigaction(SIGQUIT, &quitsa, NULL);
	err(127, "sigprocmask");
    }

    for (i = 0, curr = cmd; i < nc; curr = curr->next, i++) {
	if (pipe(p) < 0)
	    err(127, "pipe");

	if ((pid = fork()) < 0)
	    err(127, "fork");

	if (pid == 0) { /* child */

	    if (trace == 1){
		fflush(stdout);
		printf("+ %s", curr->command);
		for (j = 1; j < curr->argc + 1; j++) {
		    if (curr->argv[j] != NULL)
			printf(" %s", curr->argv[j]);
		}
		printf("\n");
	    }
	    
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

    sigaction(SIGQUIT, &quitsa, NULL);
    (void)sigprocmask(SIG_SETMASK, &omask, NULL);

    return status;
}
