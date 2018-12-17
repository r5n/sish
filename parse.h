#ifndef __PARSE_H
#define __PARSE_H

/* sish connective : >, <, >>, |, or & */
enum sish_conn { OUT, IN, APPEND, PIPE, BACKGROUND };

struct sish_command {
    char *command;
    char **argv;
    int argc;
    enum sish_conn conn;
    struct sish_command *next;
};

struct sish_command *command_new(int);
struct sish_command *parse(void);
void free_command(struct sish_command *);
void print_command(struct sish_command *);

#endif
