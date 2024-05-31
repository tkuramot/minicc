#include <stdio.h>

void no_arg() { printf("-> foo\n"); }

void one_arg(int x) { printf("-> bar: %d\n", x); }

void two_arg(int x, int y) { printf("-> bar: %d, %d\n", x, y); }

void six_arg(int a, int b, int c, int d, int e, int f) {
  printf("-> baz: %d, %d, %d, %d, %d, %d\n", a, b, c, d, e, f);
}

int one() { return 1; }

int internal_call() { return one(); }

int fibonacci(int n) {
  if (n == 0)
    return 0;
  if (n == 1)
    return 1;
  return fibonacci(n - 1) + fibonacci(n - 2);
}
