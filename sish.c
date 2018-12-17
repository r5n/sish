#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parse.h"
#include "extern.h"

#define SISH_SHELL "SHELL=sish"

static void usage(void);

int last_status;

int
main(int argc, char **argv)
{
    int ch;
    struct sish_opt *opts;
    struct sigaction sa, intsa;
    struct sish_command *comm;
    
    setprogname(argv[0]);
    last_status = 0;

    if (putenv(SISH_SHELL) == -1)
	err(127, "putenv");

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if ((opts = malloc(sizeof *opts)) == NULL) {
	fprintf(stderr, "malloc: %s\n", strerror(errno));
	exit(127);
    }

    if (sigaction(SIGINT, &sa, &intsa) == -1)
	err(1, "unable to handle SIGINT");

    while ((ch = getopt(argc, argv, "cx")) != -1) {
	switch (ch) {
	case 'c':
	    opts->command = 1;
	    break;
	case 'x':
	    opts->trace = 1;
	    break;
	default:
	    free(opts);
	    sigaction(SIGINT, &intsa, NULL);
	    usage();
	    /* NOT REACHED */
	}
    }
    argc -= optind;
    argv += optind;

    if (argc == 1 && opts->command == 1) {
	if (asprintf(&(opts->run), "%s\n", argv[0]) == -1)
	    err(127, "asprintf");
	
	if ((comm = parse_argv(opts->run)) == NULL)
	    exit(127);

	if (sish_builtin(comm, opts->trace, &last_status) == 0) {
	    last_status = sish_execute(comm, opts->trace);
	}

	free(opts->run);
	free_command(comm);
	goto cleanup;
    }

    for (;;) {
	printf("sish$ ");
	if ((comm = parse()) == NULL){
	    continue;
	}
	if (sish_builtin(comm, opts->trace, &last_status) == 0) {
	    last_status = sish_execute(comm, opts->trace);
	}
	free_command(comm);
    }

cleanup:    
    free(opts);
    sigaction(SIGINT, &intsa, NULL);
    exit(last_status);
}

static void
usage(void)
{
    fprintf(stderr, "usage: %s [-x] [-c command]\n", getprogname());
    exit(127);
}
