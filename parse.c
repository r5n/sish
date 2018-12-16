#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "parse.h"
#include "util.h"

#define CMDLEN  16
#define SEP     " \t"
#define SPECIAL "&|<>\n"

struct sish_command * command_new(int);
char **tokenize(char *, int *);
int parse_tokens(char **, int, struct sish_command *);

struct sish_command *
parse(void)
{
    int toklen;
    char *line;
    char **tokens;
    size_t size;
    ssize_t len;
    struct sish_command *comm;

    size = 0;
    line = NULL;

    comm = command_new(CMDLEN);

    if ((len = getline(&line, &size, stdin)) == -1) {
	if (errno)
	    err(EXIT_FAILURE, "getline");
	
	return NULL; /* EOF */
    }

    tokens = tokenize(line, &toklen);
    if (parse_tokens(tokens, toklen, comm) == -1)
	return NULL;

    free(line);
    
    return comm;
}

struct sish_command *
command_new(int arglen)
{
    struct sish_command *comm;

    if ((comm = malloc(sizeof *comm)) == NULL)
	err(EXIT_FAILURE, "malloc");

    if ((comm->argv = calloc(arglen, sizeof *(comm->argv))) == NULL)
	err(EXIT_FAILURE, "calloc");

    comm->command = NULL;
    comm->next = NULL;
    comm->conn = -1;
    comm->argc = 0;

    return comm;
}

enum sish_conn
get_connective(char curr, char next, int *consumed)
{
    switch (curr) {
    case '<':
	return IN;
    case '>':
	if (strncmp(&next, ">", 1) == 0) {
	    (*consumed)++;
	    return APPEND;
	}
	return OUT;
    case '|':
	return PIPE;
    case '&':
	return BACKGROUND;
    default:
	return -1;
    }
}

int
parse_tokens(char **tokens, int len, struct sish_command *comm)
{
    int i, cmd, idx, argc;
    struct sish_command *curr, *next;

    curr = comm;
    cmd = 1;
    argc = idx = 0;

    for (i = 0; i < len; i++) {
	if (cmd) {
	    if (strpbrk(tokens[i], SPECIAL) != NULL) /* invalid */
		return -1;
	    curr->command = tokens[i];
	    cmd = 0;
	    continue;
	}

	if (strpbrk(tokens[i], SPECIAL) != NULL) {
	    cmd = 1;
	    next = command_new(len);
	    curr->argc = argc;
	    curr->conn = get_connective(tokens[i][0], tokens[i+1][0], &i);
	    curr->next = next;
	    curr = next;
	} else {
	    curr->argv[idx] = tokens[i];
	    argc += 1;
	    idx += 1;
	}
    }

    curr->argc = argc;
    curr->next = NULL;

    return 1;
}

char **
tokenize(char *line, int *toklen)
{
    char *str, *tok, *sep, *tmp;
    char **tokens;
    int idx, size;;
    size_t n;

    idx = n = 0;
    size = CMDLEN;

    if ((str = strdup(line)) == NULL)
	err(EXIT_FAILURE, "strdup");

    tmp = str;

    if ((tokens = malloc(sizeof *tokens * size)) == NULL)
	err(EXIT_FAILURE, "malloc");

    while (*str != '\0') {
	n = strcspn(str, SPECIAL);

	if (strncmp(str + n, "\n", 1) != 0) {
	    if ((tok = strndup(str + n, 1)) == NULL)
		err(EXIT_FAILURE, "strndup");
	}
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
    
    *toklen = idx;
    free(tmp);    

    return tokens;
}
