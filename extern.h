#ifndef __EXTERN_H
#define __EXTERN_H

struct sish_opt {
    int command;
    int trace;
    char *run;
};

int parse_expr();

#endif
