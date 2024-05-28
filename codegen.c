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
  printf("  push rax\n");
}

void gen(Node *node) {
  static int unique_label;

  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->cont.val);
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
    printf("  push rax\n");
    return;
  } else if (node->kind == ND_FNCALL) {
    COMMENT_PRINT(
        "  # function call begin. align the stack pointer to 16 bytes");
    printf("  mov rax, rsp\n");
    printf("  mov rdi, 16\n");
    printf("  cqo\n");
    printf("  idiv rdi\n");
    printf("  sub rdi, rdx\n");
    printf("  sub rsp, rdi\n");

    /*
     * handle up to 6 arguments
     */
    COMMENT_PRINT("  # copy the arguments to the registers");
    Node *arg = node->cont.function.args;
    for (int i = 0; reg[i] && arg; ++i) {
      gen(arg);
      printf("  pop %s\n", reg[i]);
      arg = arg->next;
    }
    printf("  call %.*s\n", node->cont.function.len, node->cont.function.name);
    printf("  push rax\n");
    COMMENT_PRINT("  # function call end");
    return;
  } else if (node->kind == ND_FNDEF) {
    printf("%.*s:\n", node->cont.function.len, node->cont.function.name);
    COMMENT_PRINT("  # prologue");
    printf("  push rbp\n");     // store the base pointer to the stack
    printf("  mov rbp, rsp\n"); // set the base pointer to the stack pointer
    if (node->cont.function.locals) {
      printf("  sub rsp, %d\n", node->cont.function.locals->offset);
    }

    COMMENT_PRINT("  # copy the arguments to the local variables");
    int i = 0;
    for (Node *param = node->cont.function.params; param; param = param->next) {
      gen_lval(param);
      printf("  pop rax\n");
      printf("  mov [rax], %s\n", reg[i++]);
    }

    COMMENT_PRINT("  # function body");
    for (Node *stmt = node->cont.function.block; stmt; stmt = stmt->next) {
      gen(stmt);
    }
    printf("  pop rax\n"); // pop the return value

    COMMENT_PRINT("  # epilogue");
    printf("  mov rsp, rbp\n"); // reset the stack pointer
    printf("  pop rbp\n");      // restore the base pointer
    printf("  ret\n");          // return from the function
    return;
  } else if (node->kind == ND_ASSIGN) {
    COMMENT_PRINT("  # assignment begin");
    gen_lval(node->cont.binary.lhs);
    gen(node->cont.binary.rhs);

    /*
     * pop the value and the address from the stack
     * copy the value to the address
     */
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    COMMENT_PRINT("  # assignment end");
    return;
  } else if (node->kind == ND_IF) {
    int cur_label = unique_label++;

    /*
     * pop the value from the stack and compare it with 0
     * if the value is 0, which means false, jump to the end or else statement
     */
    COMMENT_PRINT("  # if statement begin");
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
    COMMENT_PRINT("  # if statement end");
    return;
  } else if (node->kind == ND_WHILE) {
    int cur_label = unique_label++;

    COMMENT_PRINT("  # while statement begin");
    printf(".Lbegin%d:\n", cur_label);
    gen(node->cont.loop.cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%d\n", cur_label);
    gen(node->cont.loop.then);
    printf("  jmp .Lbegin%d\n", cur_label);
    printf(".Lend%d:\n", cur_label);
    COMMENT_PRINT("  # while statement end");
    return;
  } else if (node->kind == ND_FOR) {
    int cur_label = unique_label++;

    COMMENT_PRINT("  # for statement begin");
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
    COMMENT_PRINT("  # for statement end");
    return;
  } else if (node->kind == ND_BLOCK) {
    COMMENT_PRINT("  # block statement begin");
    for (Node *stmt = node->cont.block; stmt; stmt = stmt->next) {
      gen(stmt);
      printf("  pop rax\n");
    }
    COMMENT_PRINT("  # block statement end");
    return;
  } else if (node->kind == ND_RETURN) {
    /*
     * reset the stack pointer and the base pointer
     * return from the function
     */
    COMMENT_PRINT("  # return statement");
    gen(node->cont.binary.lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }

  gen(node->cont.binary.lhs);
  gen(node->cont.binary.rhs);

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
