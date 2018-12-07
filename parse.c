#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

#define CMDLEN  10
#define TOKLEN  8
#define TOKSEP  " \t\r\n"
#define SPECIAL "&|<>"

void
tokenize(char *line)
{
    char *str, *tok;
    char **tokens;
    int idx, j;

    idx = 0;

    if ((str = strdup(line)) == NULL)
	err(EXIT_FAILURE, "strdup");

    if ((tokens = malloc(sizeof *tokens * CMDLEN)) == NULL)
	err(EXIT_FAILURE, "malloc");

    for (tok = strtok(str, TOKSEP); tok; tok = strtok(NULL, TOKSEP)) {
	tokens[idx++] = tok;
	if (idx >= CMDLEN) {
	    idx *= 2;
	    if ((tokens = realloc(tokens, sizeof *tokens *idx)) == NULL)
		err(EXIT_FAILURE, "realloc");
	}
    }
    for (j = 0; j < idx; ++j) {
	printf("token: %s\n", tokens[j]);
    }

    free(str);
}

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
	tokenize(line);
    }

    free(line);
    return 0;
}
