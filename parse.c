#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "parse.h"

#define CMDLEN  16
#define SEP     " \t"
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

void
grow(char ***tokens, int *len, int *size)
{
    if (*len >= *size) {
	*size *= 2;
	if ((*tokens = realloc(*tokens, sizeof **tokens * (*size))) == NULL)
	    err(EXIT_FAILURE, "realloc");
    }
}

char **
tokenize(char *line)
{
    char *str, *tok, *sep, *tmp;
    char **tokens;
    int idx, size;;
    int j;
    size_t n;

    idx = j = 0;
    n = 0;
    size = CMDLEN;

    if ((str = strdup(line)) == NULL)
	err(EXIT_FAILURE, "strdup");

    tmp = str;

    if ((tokens = malloc(sizeof *tokens * size)) == NULL)
	err(EXIT_FAILURE, "malloc");

    while (strlen(str) > 0) {
	n = strcspn(str, SPECIAL);

	if (strncmp(str + n, "\n", 1) != 0)
	    tok = strndup(str + n, 1);
	else
	    tok = NULL;

	*(str + n) = '\0';

	for (sep = strtok(str, SEP); sep; sep = strtok(NULL, SEP)) {

	    grow(&tokens, &idx, &size);

	    if ((tokens[idx++] = strdup(sep)) == NULL)
		err(EXIT_FAILURE, "strdup");
	}

	if (tok) {
	    grow(&tokens, &idx, &size);
	    tokens[idx++] = tok;
	}
	str += n + 1;
    }
    
    printf("\n");
    for (j = 0; j < idx; j++) {
    	printf("token: %s\n", tokens[j]);
    }

    free(tmp);

    return tokens;
}
