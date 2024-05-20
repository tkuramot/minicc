#ifndef NINECC_H
#define NINECC_H

typedef enum {
  TK_RESERVED,
  TK_NUM,
  TK_EOF,
} TokenKind;

typedef enum {
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_NUM,
  ND_EQ,
  ND_NE,
  ND_LT,
  ND_LE,
} NodeKind;

typedef struct Token Token;

typedef struct Node Node;

struct Token {
  TokenKind kind;
  Token *next;
  int val;
  char *str;
  int len;
};

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;
};

extern char *user_input;
extern Token *token;

Token *tokenize(char *p);
Node *expr();
void gen(Node *node);

#endif

