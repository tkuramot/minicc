NAME:=9cc
CFLAGS:=-std=c11 -g -static # -fsanitize=address
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

ctr:
	docker build --platform linux/amd64 -t compilerbook https://www.sigbus.info/compilerbook/Dockerfile

ctr/compile: ctr
	docker run --rm -v $(PWD):/9cc -w /9cc --platform linux/amd64 compilerbook make

ctr/test: ctr ctr/compile
	docker run --rm -v $(PWD):/9cc -w /9cc --platform linux/amd64 compilerbook make test

.PHONY: test clean

