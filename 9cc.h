#ifndef NINECC_H
#define NINECC_H

#ifdef COMMENT
#define COMMENT_PRINT(fmt, args...) printf("\n" fmt "\n", ##args)
#else
#define COMMENT_PRINT(fmt, args...)
#endif

#define MAX_CODE_LINE 100

typedef enum {
  TK_RESERVED,
  TK_IDENT,
  TK_NUM,
  TK_IF,
  TK_ELS,
  TK_WHILE,
  TK_FOR,
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
  ND_FNCALL,
  ND_FNDEF,
  ND_NUM,
  ND_IF,
  ND_ELS,
  ND_WHILE,
  ND_FOR,
  ND_BLOCK,
  ND_RETURN,
} NodeKind;

typedef struct LVar LVar;
typedef struct Token Token;
typedef union NodeContent NodeContent;
typedef struct Node Node;

struct LVar {
  LVar *next;
  char *name;
  int len;
  int offset;
};

struct Token {
  TokenKind kind;
  Token *next;
  int val;
  char *str;
  int len;
};

union NodeContent {
  struct {
    Node *lhs;
    Node *rhs;
  } binary; // +, -, *, /, ==, !=, <, <=
  struct {
    Node *cond;
    Node *then;
    Node *els;
  } conditional; // if, else
  struct {
    Node *init;
    Node *cond;
    Node *update;
    Node *then;
  } loop; // while, for
  struct {
    char *name;
    LVar *locals;
    Node *args;
    Node *params;
    Node *block;
    int len;
  } function;  // function definition and call
  Node *block; // block
  int val;     // number
  int offset;  // local variable
};

struct Node {
  NodeKind kind;
  NodeContent cont;
  Node *next;
};

extern char *user_input;
extern Token *token;
extern Node *code[MAX_CODE_LINE];

Token *tokenize(char *p);
void error(char *fmt, ...);
void program();
void gen(Node *node);

#endif
