#include <stdio.h>

#include "9cc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  user_input = argv[1];
  token = tokenize(argv[1]);
  program();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  printf("  push rbp\n");     // store the base pointer to the stack
  printf("  mov rbp, rsp\n"); // set the base pointer to the stack pointer
                              // TODO no need to allocate 208 bytes for local
                              // variables
  printf("  sub rsp, 208\n"); // allocate 208 bytes for local variables

  for (int i = 0; code[i]; ++i) {
    gen(code[i]);

    /*
     * every statement should leave the result at the top of the stack
     * pop the value from the stack to avoid stack overflow
     */
    printf("  pop rax\n");
  }

  printf("  mov rsp, rbp\n"); // reset the stack pointer
  printf("  pop rbp\n");      // restore the base pointer
  printf("  ret\n");          // return from the function
  return 0;
}
