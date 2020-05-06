#include "9cc.h"

// All local variable instances created during parsing are
// accumulated to this list.
Var *locals;

static Node *expr();
static Node *assign();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *primary();

static Var *find_var(Token *tok) {
  for(Var *var = locals; var; var = var->next)
    if(strlen(var->name) == tok->len && !strncmp(tok->str, var->name, tok->len))
      return var;
  return NULL;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
static bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
static void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "\"%s\"ではありません", op);
  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
static int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

static Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}
static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

static Node *new_unary(NodeKind kind, Node *expr) {
  Node *node = new_node(kind);
  node->lhs = expr; // use lhs as expr or return value
  return node;
}

static Node *new_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

static Node *new_var_node(Var *var) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_VAR;
  node->var = var;
  return node;
}

static Var *new_lvar(char *name) {
  Var *var = calloc(1, sizeof(Var));
  var->name = name;
  var->next = locals;
  locals = var;
  return var;
}

// stmt = "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | expr ";"
static Node *stmt() {
  if(consume("return")) {
    Node *node = new_unary(ND_RETURN, expr());
    expect(";");
    return node;
  }
  if(consume("if")) {
    Node *node = new_node(ND_IF);
    // expr and return node here
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if(consume("else"))
      node->els = stmt();
    return node;
  }
  Node *node = new_unary(ND_EXPR_STMT, expr());
  expect(";");
  return node;
}

static Node *expr() {
  return assign();
}
static Node *assign() {
  Node *node = equality();
  if(consume("="))
    node = new_binary(ND_ASSIGN, node, assign());
  return node;
}
static Node *equality() {
  Node *node = relational();

  for(;;) {
    if(consume("=="))
      node = new_binary(ND_EQ, node, relational());
    else if(consume("!="))
      node = new_binary(ND_NE, node, relational());
    else
      return node;
  }
}

static Node *relational() {
  Node *node = add();

  for(;;) {
    if (consume("<"))
      node = new_binary(ND_LT, node, add());
    else if (consume("<="))
      node = new_binary(ND_LE, node, add());
    else if (consume(">"))
      node = new_binary(ND_LT, add(), node);
    else if (consume(">="))
      node = new_binary(ND_LE, add(), node);
    else
      return node;
  }
}

static Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_binary(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_binary(ND_SUB, node, mul());
    else
      return node;
  }
}

static Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_binary(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_binary(ND_DIV, node, unary());
    else
      return node;
  }
}

static Node *unary() {
  if(consume("+"))
    return primary();
  if(consume("-"))
    return new_binary(ND_SUB, new_num(0), primary());
  return primary();
}

static Node *primary() {
  // 次のトークンが"("なら、"(" expr ")"のはず
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  if(token->kind == TK_IDENT) {
    Var *var = find_var(token);
    if(!var)
      var = new_lvar(strndup(token->str, token->len));
    token = token->next;
    return new_var_node(var);
  }

  Node *node = new_num(expect_number());
  return node;
}

Function *parse() {
  Node head = {};
  Node *cur = &head;
  while(token->kind != TK_EOF)
    cur = cur->next = stmt();

  Function *prog = calloc(1, sizeof(Function));
  prog->node = head.next;
  prog->locals = locals;

  return prog;
}
