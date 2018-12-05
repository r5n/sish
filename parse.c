#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

int
parse_expr(void)
{
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    if ((linelen = getline(&line, &linecap, stdin)) > 0) {
	if (strncmp(line, "exit", 4) == 0)
	    return -1;
	fwrite(line, linelen, 1, stdout);
    }

    free(line);
    return 0;
}
