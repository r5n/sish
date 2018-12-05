CFLAGS += -Wall -Werror -Wextra -Wno-unknown-pragmas -pedantic -std=c99 -g
LDLIBS = -lm

sish: sish.o parse.o
	$(CC) $(CFLAGS) $(LDLIBS) -o $@ $^ $>

clean:
	rm *.o
	rm sish

.PHONY: clean
