#ifndef __EXTERN_H
#define __EXTERN_H

struct sish_opt {
    int command;
    int trace;
    char *run;
};

int sish_builtin(struct sish_command *, int);
int sish_execute(struct sish_command *, int);

#endif
