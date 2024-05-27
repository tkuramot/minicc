#include <stdio.h>

void foo() { printf("-> foo\n"); }

void bar(int x, int y) { printf("-> bar: %d, %d\n", x, y); }

void baz(int a, int b, int c, int d, int e, int f) {
  printf("-> baz: %d, %d, %d, %d, %d, %d\n", a, b, c, d, e, f);
}
