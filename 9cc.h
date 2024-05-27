#ifndef NINECC_H
#define NINECC_H

typedef enum {
  TK_RESERVED,
  TK_IDENT,
  TK_NUM,
	TK_IF,
  TK_RETURN,
  TK_EOF,
} TokenKind;

typedef enum {
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_EQ,
  ND_NE,
  ND_LT,
  ND_LE,
  ND_ASSIGN,
  ND_LVAR,
  ND_NUM,
	ND_IF,
  ND_RETURN,
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
  int offset;
};

extern char *user_input;
extern Token *token;
extern Node *code[100];

Token *tokenize(char *p);
void error(char *fmt, ...);
void program();
void gen(Node *node);

#endif
