#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
match(const char *src, const char *dest)
{
    int slen, dlen;

    if (src == NULL)
        return 0;

    slen = strlen(src);
    dlen = strlen(dest);

    if (slen == dlen && (strncmp(src, dest, slen)) == 0)
        return 1;

    return 0;
}

void
grow(char ***buf, int *len, int *size)
{
    if (*len >= *size) {
        *size *= 2;
        if ((*buf = realloc(*buf, sizeof **buf * (*size))) == NULL) {
            fprintf(stderr, "realloc: %s\n", strerror(errno));
            exit(127);
        }
    }
}
