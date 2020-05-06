#include "9cc.h"

static int labelseq = 1;

static void gen_addr(Node *node) {
  if(node->kind == ND_VAR) {
    printf("  lea rax, [rbp-%d]\n", node->var->offset);
    printf("  push rax\n");
  }
}

static void load(void) {
  printf("  pop rax\n");
  printf("  mov rax, [rax]\n");
  printf("  push rax\n");
}

static void store(void) {
  printf("  pop rax\n");
  printf("  pop rdi\n");
  printf("  mov [rax], rdi\n");
  printf("  push rdi\n");
}

static void gen_expr(Node *node) {
  switch(node->kind) {
    case ND_NUM:
      printf("  push %lu\n", node->val);
      return;
    case ND_VAR:
      gen_addr(node);
      load();
      return;
    case ND_ASSIGN:
      gen_expr(node->rhs);
      gen_addr(node->lhs);
      store();
      return;
  }

  gen_expr(node->lhs);
  gen_expr(node->rhs);

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
  }

  printf("  push rax\n");
}

static void gen_stmt(Node *node) {
  switch(node->kind) {
    case ND_IF:
      gen_expr(node->cond);
      int seq = labelseq++;
      if(node->els) {
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .L.else.%d\n", seq);
        gen_stmt(node->then);
        printf("  jmp .L.end.%d\n", seq);
        printf(".L.else.%d:\n", seq);
        gen_stmt(node->els);
        printf(".L.end.%d:\n", seq);
      } else {
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .L.end.%d\n", seq);
        gen_stmt(node->then);
        printf(".L.end.%d:\n", seq);
      }
      return;
    case ND_FOR: {
      int seq = labelseq++;
      if(node->init)
        gen_stmt(node->init);
      printf(".L.begin.%d:\n", seq);
      if(node->cond) {
        gen_expr(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .L.end.%d\n", seq);
      }
      gen_stmt(node->then);
      if(node->inc)
        gen_stmt(node->inc);
      printf("  jmp .L.begin.%d\n", seq);
      printf(".L.end.%d:\n", seq);
      return;
    }
    case ND_WHILE: {
      int seq = labelseq++;
      printf(".L.begin.%d:\n", seq);

      gen_expr(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .L.end.%d\n", seq);

      gen_stmt(node->then);
      printf("  jmp .L.begin.%d\n", seq);
      printf(".L.end.%d:\n", seq);
      return;
    }
    case ND_RETURN:
      gen_expr(node->lhs);
      printf("  pop rax\n");
      printf("  jmp .L.return\n");
      return;
    case ND_EXPR_STMT:
      gen_expr(node->lhs);
      printf("  pop rax\n");
      return;
    default:
      error("invalid statement");
  }
}

void codegen(Function *prog) {

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", prog->stack_size);

  for(Node *n = prog->node; n; n = n->next) {
    gen_stmt(n);
  }
  printf(".L.return:\n");
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}
