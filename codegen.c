#include <stdio.h>

#include "9cc.h"

// the registers to pass the arguments
static const char *reg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9", NULL};

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
  printf("  sub rax, %d\n", node->cont.offset);
  printf("  push rax\n\n");
}

void gen(Node *node) {
  static int unique_label;

  if (node->kind == ND_NUM) {
    printf("  push %d\n\n", node->cont.val);
    return;
  } else if (node->kind == ND_LVAR) {
    /*
     * get the local variable value from the stack
     * pop from the stack to the rax register
     * dereference the rax register and push the value to the stack
     */
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n\n");
    return;
  } else if (node->kind == ND_FNCALL) {
    /*
     * handle up to 6 arguments
     */
    COMMENT("prepare for function call");
    COMMENT("copy the arguments on the stack to the registers");
    Node *arg = node->cont.function.args;
    for (int i = 0; reg[i] && arg; ++i) {
      gen(arg);
      printf("  pop %s\n", reg[i]);
      arg = arg->next;
    }

    COMMENT("align the stack pointer to 16 bytes");
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  and rsp, -16\n");
    printf("  call %.*s\n", node->cont.function.len, node->cont.function.name);

    COMMENT("restore the stack pointer and the base pointer");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  push rax\n\n");
    return;
  } else if (node->kind == ND_FNDEF) {
    printf("%.*s:\n", node->cont.function.len, node->cont.function.name);
    COMMENT("prologue");
    printf("  push rbp\n");     // store the base pointer to the stack
    printf("  mov rbp, rsp\n"); // set the base pointer to the stack pointer
    if (node->cont.function.locals) {
      printf("  sub rsp, %d\n", node->cont.function.locals->offset);
    }
    printf("\n");

    /*
     * handle up to 6 parameters
     */
    COMMENT("copy the arguments in the registers to the local variables");
    Node *param = node->cont.function.params;
    for (int i = 0; reg[i] && param; ++i) {
      gen_lval(param);
      printf("  pop rax\n");
      printf("  mov [rax], %s\n", reg[i++]);
      param = param->next;
    }

    COMMENT("function body");
    for (Node *stmt = node->cont.function.block; stmt; stmt = stmt->next) {
      gen(stmt);
    }
    printf("  pop rax\n\n"); // pop the return value

    COMMENT("epilogue");
    printf("  mov rsp, rbp\n"); // reset the stack pointer
    printf("  pop rbp\n");      // restore the base pointer
    printf("  ret\n\n");        // return from the function
    return;
  } else if (node->kind == ND_ASSIGN) {
    COMMENT("assignment");
    gen_lval(node->cont.binary.lhs);
    gen(node->cont.binary.rhs);

    /*
     * pop the value and the address from the stack
     * copy the value to the address
     */
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n\n");
    return;
  } else if (node->kind == ND_IF) {
    int cur_label = unique_label++;

    /*
     * pop the value from the stack and compare it with 0
     * if the value is 0, which means false, jump to the end or else statement
     */
    COMMENT("if statement");
    gen(node->cont.conditional.cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    if (node->cont.conditional.els) {
      printf("  je .Lelse%d\n", cur_label);
      gen(node->cont.conditional.then);
      printf("  jmp .Lend%d\n", cur_label);
      printf(".Lelse%d:\n", cur_label);
      gen(node->cont.conditional.els);
      printf(".Lend%d:\n", cur_label);
    } else {
      printf("  je .Lend%d\n", cur_label);
      gen(node->cont.conditional.then);
      printf(".Lend%d:\n", cur_label);
    }
    return;
  } else if (node->kind == ND_WHILE) {
    int cur_label = unique_label++;

    COMMENT("while statement");
    printf(".Lbegin%d:\n", cur_label);
    gen(node->cont.loop.cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%d\n", cur_label);
    gen(node->cont.loop.then);
    printf("  jmp .Lbegin%d\n", cur_label);
    printf(".Lend%d:\n", cur_label);
    return;
  } else if (node->kind == ND_FOR) {
    int cur_label = unique_label++;

    COMMENT("for statement");
    if (node->cont.loop.init) {
      gen(node->cont.loop.init);
    }
    printf(".Lbegin%d:\n", cur_label);
    if (node->cont.loop.cond) {
      gen(node->cont.loop.cond);
    }
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%d\n", cur_label);
    if (node->cont.loop.then) {
      gen(node->cont.loop.then);
    }
    if (node->cont.loop.update) {
      gen(node->cont.loop.update);
    }
    printf("  jmp .Lbegin%d\n", cur_label);
    printf(".Lend%d:\n", cur_label);
    return;
  } else if (node->kind == ND_BLOCK) {
    COMMENT("block statement");
    for (Node *stmt = node->cont.block; stmt; stmt = stmt->next) {
      gen(stmt);
      printf("  pop rax\n");
    }
    printf("\n");
    return;
  } else if (node->kind == ND_RETURN) {
    /*
     * reset the stack pointer and the base pointer
     * return from the function
     */
    COMMENT("return statement");
    gen(node->cont.binary.lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n\n");
    return;
  }

  gen(node->cont.binary.lhs);
  gen(node->cont.binary.rhs);

  COMMENT("binary operator");
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

  printf("  push rax\n\n");
}
