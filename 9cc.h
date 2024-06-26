#ifndef NINECC_H
#define NINECC_H

#ifdef WITH_COMMENT
#define COMMENT(fmt, args...) printf("  # " fmt "\n", ##args)
#else
#define COMMENT(fmt, args...)
#endif

#define MAX_CODE_LINE 100

typedef enum {
  TK_RESERVED,
  TK_TYPE,
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
  ND_ADDR,
  ND_DEREF,
  ND_IF,
  ND_ELS,
  ND_WHILE,
  ND_FOR,
  ND_BLOCK,
  ND_RETURN,
} NodeKind;

typedef struct LVar LVar;
typedef struct Type Type;
typedef struct Token Token;
typedef union NodeContent NodeContent;
typedef struct Node Node;

struct LVar {
  LVar *next;
  char *name;
  int len;
  int offset;
};

struct Type {
  struct Type *ptr_to;
  enum { INT, PTR } kind;
  int size;
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
    Node *operand;
  } unary; // *, &
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
    Node *args;
    Node *params;
    Node *block;
    Type *return_type;
    int stack_size;
    int len;
  } function; // function definition and call
  struct {
    Type *type;
    int offset;
  } lvar;      // local variable
  Node *block; // block
  int val;     // number
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
