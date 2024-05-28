#include <stdio.h>

#include "9cc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: ./%s <input>\n", argv[0]);
    return 1;
  }

  user_input = argv[1];
  token = tokenize(argv[1]);
  program();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  for (int i = 0; code[i]; ++i) {
    gen(code[i]);
  }
  return 0;
}
