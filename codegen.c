#include <stdio.h>

#include "9cc.h"

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR) {
    error("left value is not a variable");
  }

  /*
   * access the local variable by the offset from the base pointer
   * copy the base pointer to the rdi register
   * subtract the offset from the rdi register
   * push the rdi register to the stack
   */
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen(Node *node) {
  static int unique_label;

  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  } else if (node->kind == ND_LVAR) {
    gen_lval(node);

    /*
     * get the local variable value from the stack
     * pop from the stack to the rax register
     * dereference the rax register and push the value to the stack
     */
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  } else if (node->kind == ND_ASSIGN) {
    gen_lval(node->lhs);
    gen(node->rhs);

    /*
     * pop the value and the address from the stack
     * copy the value to the address
     */
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  } else if (node->kind == ND_IF) {
    gen(node->cond);

    /*
     * pop the value from the stack and compare it with 0
     * if the value is 0, which means false, jump to the end or else statement
     */
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    if (node->els) {
      printf("  je .Lelse%d\n", unique_label);
      gen(node->then);
      printf("  jmp .Lend%d\n", unique_label);
      printf(".Lelse%d:\n", unique_label);
      gen(node->els);
      printf(".Lend%d:\n", unique_label);
    } else {
      printf("  je .Lend%d\n", unique_label);
      gen(node->then);
      printf(".Lend%d:\n", unique_label);
    }

    unique_label++;
    return;
  } else if (node->kind == ND_RETURN) {
    gen(node->lhs);

    /*
     * reset the stack pointer and the base pointer
     * return from the function
     */
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  default:
    break;
  }

  printf("  push rax\n");
}
