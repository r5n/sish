#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

static void usage(void);

int
main(int argc, char **argv)
{
    int ch, n;
    struct sish_opt *opts;
    
    setprogname(argv[0]);
    
    if ((opts = malloc(sizeof(struct sish_opt))) == NULL) {
	fprintf(stderr, "%s\n", strerror(errno));
	exit(EXIT_FAILURE);
    }

    while ((ch = getopt(argc, argv, "cx")) != -1) {
	switch (ch) {
	case 'c':
	    opts->command = 1;
	    break;
	case 'x':
	    opts->trace = 1;
	    break;
	default:
	    usage();
	    /* NOT REACHED */
	}
    }
    argc -= optind;
    argv += optind;

    if (argc == 1) {
	opts->run = argv[0];
    }

    do {
	printf("sish$ ");
	n = parse_expr();
    } while (n != -1);

    free(opts);
}

static void
usage(void)
{
    fprintf(stderr, "usage: %s [-x] [-c command]\n", getprogname());
    exit(EXIT_FAILURE);
}
