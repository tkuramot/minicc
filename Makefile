CFLAGS=-std=c11 -g -static

9cc: 9cc.c

test: 9cc
	./test.sh

clean:
	$(RM) -f 9cc *.o *.~ tmp*

ctr:
	docker build --platform linux/amd64 -t compilerbook https://www.sigbus.info/compilerbook/Dockerfile

ctr/compile: ctr
	docker run --rm -v $(PWD):/9cc -w /9cc --platform linux/amd64 compilerbook make

ctr/test: ctr ctr/compile
	docker run --rm -v $(PWD):/9cc -w /9cc --platform linux/amd64 compilerbook make test

.PHONY: test clean

