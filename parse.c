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
#define SYN_ERR "syntax error near unexpected token"

struct sish_command * command_new(int);
char **tokenize(char *, int *);
int parse_tokens(char **, int, struct sish_command *);
void free_command(struct sish_command *);
int validate(struct sish_command *);

void
print_command(struct sish_command *comm)
{
    int i;
    struct sish_command *tmp;

    for (tmp = comm; tmp; tmp = tmp->next) {
	printf("%s ", tmp->command);
	for (i = 1; i < tmp->argc + 1; i++) {
	    printf("%s ", tmp->argv[i]);
	}
	printf("\tOUT to %s", tmp->stdout);
	printf("\tIN from %s", tmp->stdin);
	if (tmp->append)
	    printf("\tAPPEND to %s", tmp->stdout);	
	switch (tmp->conn) {
	case OUT:
	    break;
	case IN:
	    break;
	case APPEND:

	    break;
	case PIPE:
	    printf("PIPE");
	    break;
	case BACKGROUND:
	    printf("BACKGROUND");
	    break;
	default:
	    printf(".");
	    break;
	}
	printf("\n");
    }
}


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

	comm->command = "exit";
	comm->stdin = NULL;
	comm->stdout = NULL;
	comm->argc = 0;
	comm->argv = NULL;
	comm->next = NULL;

	return comm;
    }

    tokens = tokenize(line, &toklen);
    if (parse_tokens(tokens, toklen, comm) == -1)
	return NULL;

    free(line);
    
    return comm;
}

void
free_command(struct sish_command *comm)
{
    int i;
    struct sish_command *head, *tmp;

    head = comm;
    while (head != NULL) {
	tmp = head;
	head = head->next;

	free(tmp->command);
	for (i = 1; i < tmp->argc + 1; i++) {
	    free(tmp->argv[i]);
	}
    }
    free(comm->stdin);
    free(comm->stdout);
    head = NULL;
    free(comm);
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
    comm->append = 0;
    comm->stdin = comm->stdout = NULL;

    return comm;
}

enum sish_conn
get_connective(char curr, char next, int *consumed)
{
    enum sish_conn typ;
    
    switch (curr) {
	printf("curr: %c\n", curr);
    case '<':
	typ = IN;
	break;
    case '>':
	if (next == '>') {
	    *consumed += 1;
	    typ = APPEND;
	    break;
	}
	typ = OUT;
	break;
    case '|':
	typ = PIPE;
	break;
    case '&':
	typ = BACKGROUND;
	break;
    default:
	typ = -1;
	break;
    }
    return typ;
}

int
parse_tokens(char **tokens, int len, struct sish_command *comm)
{
    int i, cmd, idx, argc;
    struct sish_command *curr, *next;

    curr = comm;
    cmd = 1;
    argc = idx = 0;

    if (len == 1 && strncmp(tokens[0], "\n", 1) == 0)
	return -1;

    for (i = 0; i < len; i++) {

	if (i == len - 1) {
	    if (strncmp(tokens[i], "\n", 1) != 0) {
		fprintf(stderr, "%s: " SYN_ERR " `%s'\n",
			getprogname(), tokens[i]);
		return -1;
	    }
	    break;
	}
	
	if (cmd) {
	    if (strpbrk(tokens[i], SPECIAL) != NULL) { /* invalid */
		fprintf(stderr, "%s: " SYN_ERR " `%s'\n",
			getprogname(),
			(*tokens[i] == '\n' ? "newline" : tokens[i]));
		return -1;
	    }
	    curr->command = tokens[i];
	    cmd = 0;
	    curr->argv[0] = curr->command;
	    idx = 1;
	    continue;
	}

	if (strpbrk(tokens[i], SPECIAL) != NULL) {
	    cmd = 1;
	    curr->argc = argc;
	    curr->conn = get_connective(tokens[i][0], tokens[i+1][0], &i);

	    if (strpbrk(tokens[i+1], SPECIAL) != NULL) {
		fprintf(stderr, "%s: " SYN_ERR " `%s'\n",
			getprogname(),
			(match(tokens[i+1], "\n") ? "newline" : tokens[i+1]));
		return -1;
	    }

	    switch (curr->conn) {
	    case IN:
		cmd = 0;
		curr->stdin = tokens[i+1];
		i++;
		break;
	    case OUT:
		cmd = 0;
		curr->stdout = tokens[i+1];
		i++;
		break;
	    case APPEND:
		cmd = 0;
		curr->append = 1;
		curr->stdout = tokens[i+1];
		i++;
		break;
	    default:
		next = command_new(len + 2);
		curr->next = next;
		curr = next;
		break;
	    }
	} else {
	    curr->argv[idx] = tokens[i];
	    argc += 1;
	    idx += 1;
	}
    }

    curr->argv[idx] = NULL;
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

	tok = strndup(str + n, 1);

	*(str + n) = '\0';

	for (sep = strtok(str, SEP); sep; sep = strtok(NULL, SEP)) {

	    grow(&tokens, &idx, &size);

	    if ((tokens[idx++] = strdup(sep)) == NULL)
		err(EXIT_FAILURE, "strdup");
	}

	grow(&tokens, &idx, &size);
	tokens[idx++] = tok;
	
	str += n + 1;
    }

    *toklen = idx;
    free(tmp);    

    return tokens;
}
