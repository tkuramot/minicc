NAME:=9cc
CFLAGS:=-std=c11 -g -static
# CFLAGS:=-std=c11 -g -fsanitize=address
SRCS:=$(wildcard *.c)
OBJS:=$(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LDFLAGS)

%.o: %.c 9cc.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ -c $<

test: $(NAME)
	./test.sh

clean:
	$(RM) -f $(NAME) *.o *.~ tmp*

re: clean all

ctr:
	docker build --platform linux/amd64 -t compilerbook https://www.sigbus.info/compilerbook/Dockerfile

ctr/compile:
	docker run --rm -v $(PWD):/9cc -w /9cc --platform linux/amd64 compilerbook make

ctr/test: ctr/compile
	docker run --rm -v $(PWD):/9cc -w /9cc --platform linux/amd64 compilerbook make test

ctr/in:
	docker run --rm -it -v $(PWD):/9cc -w /9cc --platform linux/amd64 compilerbook /bin/bash

.PHONY: test clean

