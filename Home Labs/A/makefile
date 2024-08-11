CC = gcc
CFLAGS = -Wall -Wextra -g

encoder: encoder.c
	$(CC) $(CFLAGS) encoder.c -o encoder

.PHONY: clean
clean:
	rm -f encoder

-PHONY: all
all: encoder