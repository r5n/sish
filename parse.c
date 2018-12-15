#include <ctype.h>
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
#define SPECIAL " &|<>\n"

void
tokenize(char *line)
{
    char *str, *tok;
    char **tokens;
    int idx;

    idx = 0;

    if ((str = strdup(line)) == NULL)
	err(EXIT_FAILURE, "strdup");

    if ((tokens = malloc(sizeof *tokens * CMDLEN)) == NULL)
	err(EXIT_FAILURE, "malloc");

    for (tok = strtok(str, SPECIAL); tok; tok = strtok(NULL, TOKSEP)) {
	tokens[idx++]= tok;
	if (idx >= CMDLEN) {
	    idx *= 2;
	    if ((tokens = realloc(tokens, sizeof *tokens * idx)) == NULL)
		err(EXIT_FAILURE, "realloc");
	}
    }

    free(str);
}

int
parse_expr(void)
{
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    if ((linelen = getline(&line, &linecap, stdin)) <= 0)
	return -1;

    if ((strncmp(line, "exit", 4)) == 0)
	return -1;

    printf("%s", line);
    tokenize(line);

    
    free(line);
    return 0;
}
