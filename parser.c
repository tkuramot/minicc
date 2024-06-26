#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

char *user_input;
Token *token;
Node *code[MAX_CODE_LINE];
LVar *locals;

static Node *expr();

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

bool ft_isalpha(char c) { return isalpha(c) || c == '_'; }

bool ft_isalnum(char c) { return ft_isalpha(c) || isdigit(c); }

bool next_token_is(TokenKind kind) { return token->kind == kind; }

Token *consume(TokenKind kind) {
  if (token->kind != kind) {
    return NULL;
  }
  Token *tok = token;
  token = token->next;
  return tok;
}

Token *consume_op(char *op) {
  if (token->kind != TK_RESERVED || token->len != strlen(op) ||
      memcmp(token->str, op, token->len) != 0) {
    return NULL;
  }
  Token *tok = token;
  token = token->next;
  return tok;
}

void expect(char *op) {
  if (token->kind != TK_RESERVED || token->len != strlen(op) ||
      memcmp(token->str, op, token->len) != 0) {
    error_at(token->str, "\"%s\" expected", op);
  }
  token = token->next;
}

int expect_number() {
  if (token->kind != TK_NUM) {
    error_at(token->str, "is not a number");
  }
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

bool startwith(char *src, char *target) {
  return memcmp(src, target, strlen(target)) == 0;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (strncmp(p, "int", 3) == 0 && !ft_isalnum(p[3])) {
      cur = new_token(TK_TYPE, cur, p, 3);
      p += 3;
      continue;
    }

    if (strncmp(p, "if", 2) == 0 && !ft_isalnum(p[2])) {
      cur = new_token(TK_IF, cur, p, 2);
      p += 2;
      continue;
    } else if (strncmp(p, "else", 4) == 0 && !ft_isalnum(p[4])) {
      cur = new_token(TK_ELS, cur, p, 4);
      p += 4;
      continue;
    } else if (strncmp(p, "while", 5) == 0 && !ft_isalnum(p[5])) {
      cur = new_token(TK_WHILE, cur, p, 5);
      p += 5;
      continue;
    } else if (strncmp(p, "for", 3) == 0 && !ft_isalnum(p[3])) {
      cur = new_token(TK_FOR, cur, p, 3);
      p += 3;
      continue;
    } else if (strncmp(p, "return", 6) == 0 && !ft_isalnum(p[6])) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    if (startwith(p, "==") || startwith(p, "!=") || startwith(p, "<=") ||
        startwith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
        *p == ')' || *p == '{' || *p == '}' || *p == '<' || *p == '>' ||
        *p == '=' || *p == ',' || *p == ';' || *p == '&') {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (ft_isalpha(*p)) {
      cur = new_token(TK_IDENT, cur, p, 0);
      char *q = p;
      while (ft_isalnum(*p))
        p++;
      cur->len = p - q;
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    error_at(p, "failed to tokenize");
  }

  cur = new_token(TK_EOF, cur, p, 0);
  return head.next;
}

LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next) {
    if (var->len == tok->len && memcmp(var->name, tok->str, var->len) == 0) {
      return var;
    }
  }
  return NULL;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->cont.binary.lhs = lhs;
  node->cont.binary.rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->cont.val = val;
  return node;
}

Type *type() {
  Type *typ = calloc(1, sizeof(Type));

  Token *tok = consume(TK_TYPE);
  if (!tok) {
    error_at(token->str, "type expected");
  }

  if (memcmp(tok->str, "int", 3) == 0) {
    typ->kind = INT;
    typ->size = 8;
  } else {
    error_at(tok->str, "unknown type");
  }

  while (consume_op("*")) {
    Type *ptr = calloc(1, sizeof(Type));
    ptr->kind = PTR;
    ptr->size = 8;
    ptr->ptr_to = typ;
    typ = ptr;
  }

  return typ;
}

Node *args() {
  Node head;
  head.next = NULL;
  Node *cur = &head;

  int i = 0;
  do {
    cur->next = expr();
    cur = cur->next;
  } while (consume_op(","));
  return head.next;
}

Node *params() {
  Node head;
  head.next = NULL;
  Node *cur = &head;

  int i = 0;
  Token *tok;
  do {
    Type *typ = type();
    tok = consume(TK_IDENT);
    if (!tok) {
      error_at(token->str, "parameter expected");
    }

    LVar *lvar = find_lvar(tok);
    if (lvar) {
      error_at(tok->str, "duplicate identifier");
    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = locals ? locals->offset + typ->size : typ->size;

      cur->next = new_node(ND_LVAR, NULL, NULL);
      cur->next->cont.lvar.offset = lvar->offset;
      cur->next->cont.lvar.type = typ;
      cur = cur->next;
      locals = lvar;
    }
  } while (consume_op(","));
  return head.next;
}

Node *primary() {
  if (consume_op("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  // parse variable declaration
  if (next_token_is(TK_TYPE)) {
    Node *node = new_node(ND_LVAR, NULL, NULL);
    Type *typ = type();

    Token *tok = consume(TK_IDENT);
    LVar *lvar = find_lvar(tok);
    if (lvar) {
      error_at(tok->str, "duplicate identifier");
    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = locals ? locals->offset + typ->size : typ->size;
      node->cont.lvar.offset = lvar->offset;
      node->cont.lvar.type = typ;
      locals = lvar;
    }
    return node;
  }

  // parse function call or variable reference
  Token *tok = consume(TK_IDENT);
  if (tok) {
    if (consume_op("(")) {
      Node *node = new_node(ND_FNCALL, NULL, NULL);
      node->cont.function.name = tok->str;
      node->cont.function.len = tok->len;

      if (!consume_op(")")) {
        node->cont.function.args = args();
        expect(")");
      }
      return node;
    } else {
      Node *node = new_node(ND_LVAR, NULL, NULL);

      LVar *lvar = find_lvar(tok);
      if (lvar) {
        node->cont.lvar.offset = lvar->offset;
      } else {
        error_at(tok->str, "undefined identifier");
      }
      return node;
    }
  }

  return new_node_num(expect_number());
}

Node *unary() {
  if (consume_op("+")) {
    return primary();
  }
  if (consume_op("-")) {
    return new_node(ND_SUB, new_node_num(0), primary());
  }
  if (consume_op("&")) {
    Node *node = new_node(ND_ADDR, NULL, NULL);
    node->cont.unary.operand = unary();
    return node;
  }
  if (consume_op("*")) {
    Node *node = new_node(ND_DEREF, NULL, NULL);
    node->cont.unary.operand = unary();
    return node;
  }
  return primary();
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume_op("*")) {
      node = new_node(ND_MUL, node, unary());
    } else if (consume_op("/")) {
      node = new_node(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume_op("+")) {
      node = new_node(ND_ADD, node, mul());
    } else if (consume_op("-")) {
      node = new_node(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume_op("<="))
      node = new_node(ND_LE, node, add());
    else if (consume_op(">="))
      node = new_node(ND_LE, add(), node);
    else if (consume_op("<"))
      node = new_node(ND_LT, node, add());
    else if (consume_op(">"))
      node = new_node(ND_LT, add(), node);
    else
      return node;
  }
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume_op("==")) {
      node = new_node(ND_EQ, node, relational());
    } else if (consume_op("!=")) {
      node = new_node(ND_NE, node, relational());
    } else {
      return node;
    }
  }
}

Node *assign() {
  Node *node = equality();

  for (;;) {
    if (consume_op("=")) {
      node = new_node(ND_ASSIGN, node, assign());
    } else {
      return node;
    }
  }
}

Node *expr() { return assign(); }

Node *stmt() {
  Node *node;
  if (consume(TK_IF)) {
    node = new_node(ND_IF, NULL, NULL);

    expect("(");
    node->cont.conditional.cond = expr();
    expect(")");
    node->cont.conditional.then = stmt();
    if (consume(TK_ELS)) {
      node->cont.conditional.els = stmt();
    }
  } else if (consume(TK_ELS)) {
    node = new_node(ND_ELS, NULL, NULL);
    node->cont.conditional.then = stmt();
  } else if (consume(TK_WHILE)) {
    node = new_node(ND_WHILE, NULL, NULL);

    expect("(");
    node->cont.loop.cond = expr();
    expect(")");
    node->cont.loop.then = stmt();
  } else if (consume(TK_FOR)) {
    node = new_node(ND_FOR, NULL, NULL);

    expect("(");
    if (!consume_op(";")) {
      node->cont.loop.init = expr();
      expect(";");
    }
    if (!consume_op(";")) {
      node->cont.loop.cond = expr();
      expect(";");
    }
    if (!consume_op(")")) {
      node->cont.loop.update = expr();
      expect(")");
    }
    node->cont.loop.then = stmt();
  } else if (consume_op("{")) {
    node = new_node(ND_BLOCK, NULL, NULL);

    Node head;
    head.next = NULL;
    Node *cur = &head;
    while (!consume_op("}")) {
      cur->next = stmt();
      cur = cur->next;
    }
    node->cont.block = head.next;
  } else if (consume(TK_RETURN)) {
    node = new_node(ND_RETURN, expr(), NULL);
    expect(";");
  } else {
    node = expr();
    expect(";");
  }
  return node;
}

Node *func() {
  Node *node = new_node(ND_FNDEF, NULL, NULL);

  locals = NULL; // clear previous local variables

  // parse function definition
  Type *typ = type();
  node->cont.function.return_type = typ;
  Token *tok = consume(TK_IDENT);
  if (!tok) {
    error_at(token->str, "function definition expected");
  }
  node->cont.function.name = tok->str;
  node->cont.function.len = tok->len;

  // parse parameters
  expect("(");
  if (!consume_op(")")) {
    node->cont.function.params = params();
    expect(")");
  }

  // parse function body
  expect("{");
  Node head;
  head.next = NULL;
  Node *cur = &head;
  while (!consume_op("}")) {
    cur->next = stmt();
    cur = cur->next;
  }
  node->cont.function.block = head.next;
  node->cont.function.stack_size = locals ? locals->offset : 0;
  return node;
}

void program() {
  int i = 0;
  while (!at_eof()) {
    code[i++] = func();
  }
  code[i] = NULL;
}
