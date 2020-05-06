#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// tokenize.c
//

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_IDENT,    // 識別子
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

// トークン型
typedef struct Token Token;
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
  int len;        // トークンの長さ
};
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
Token *tokenize(char *p);

// 現在着目しているトークン
Token *token;

//
// parse.c
//

// Local variable
typedef struct Var Var;
struct Var {
  Var *next;
  char *name;
  int offset;
};

// AST node
typedef enum {
  ND_ADD,       // +
  ND_SUB,       // -
  ND_MUL,       // *
  ND_DIV,       // *
  ND_EQ,        // ==
  ND_NE,        // !=
  ND_LT,        // <
  ND_LE,        // <=
  ND_ASSIGN,    // =
  ND_RETURN,    // "return"
  ND_IF,        // "if"
  ND_FOR,       // "for"
  ND_EXPR_STMT, //Expression statement
  ND_VAR,       // Variable
  ND_NUM,       // 整数
} NodeKind;

// 抽象構文木のノードの型
typedef struct Node Node;
struct Node {
  NodeKind kind; // ノードの型
  Node *next;    // Next node
  Node *lhs;     // 左辺
  Node *rhs;     // 右辺

  // "if" statement
  Node *cond;
  Node *then;
  Node *els;

  // "for" statement
  Node *init;
  // also use cond
  Node *inc;
  // also use then

  Var *var;     // use if kind == ND_VAR
  long val;       // kindがND_NUMの場合のみ使う
};

typedef struct Function Function;
struct Function {
  Node *node;
  Var *locals;
  int stack_size;
};

Function *parse();

//
// codegen.c
//

void codegen(Function *prog);
