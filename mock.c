#include <stdio.h>

void no_arg() { printf("-> foo\n"); }

void one_arg(int x) { printf("-> bar: %d\n", x); }

void two_arg(int x, int y) { printf("-> bar: %d, %d\n", x, y); }

void six_arg(int a, int b, int c, int d, int e, int f) {
  printf("-> baz: %d, %d, %d, %d, %d, %d\n", a, b, c, d, e, f);
}
