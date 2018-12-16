#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "parse.h"

#define CMDLEN  10
#define TOKLEN  8
#define TOKSEP  " \t"
#define SPECIAL "&|<>\n"

struct sish_command *
parse(void)
{
    char *line;
    char **tokens;
    size_t size;
    ssize_t len;
    struct sish_command *comm;

    size = 0;
    line = NULL;

    if ((comm = malloc(sizeof *comm)) == NULL)
	err(EXIT_FAILURE, "malloc");

    if ((len = getline(&line, &size, stdin)) == -1) {
	if (errno)
	    err(EXIT_FAILURE, "getline");
	
	return NULL; /* EOF */
    }

    tokens = tokenize(line);

    free(line);
    return comm;
}

char **
tokenize(char *line)
{
    char *str, *tok, *sep, *ret, *brk;
    char **tokens;
    int idx, j;

    idx = j = 0;

    if ((str = strdup(line)) == NULL)
	err(EXIT_FAILURE, "strdup");

    if ((tokens = malloc(sizeof *tokens * CMDLEN)) == NULL)
	err(EXIT_FAILURE, "malloc");

    /* split line by special characters first
     * and then split each resulting token by whitespace */

    for (tok = strtok_r(str, SPECIAL, &ret); tok;
	 tok = strtok_r(NULL, SPECIAL, &ret)) {

	for (sep = strtok_r(tok, TOKSEP, &brk); sep;
	     sep = strtok_r(NULL, TOKSEP, &brk)) {

	    if (idx >= CMDLEN) {
		idx *= 2;
		if ((tokens = realloc(tokens, sizeof *tokens * idx)) == NULL)
		    err(EXIT_FAILURE, "realloc");
	    }

	    if ((tokens[idx++] = strdup(sep)) == NULL)
		err(EXIT_FAILURE, "strdup");
	}
    }
    
    for (j = 0; j < idx; j++) {
	printf("token: %s\n", tokens[j]);
    }
    
    return tokens;
}
