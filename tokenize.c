#include "9cc.h"

static char *current_input;

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}
// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - current_input;
  fprintf(stderr, "%s\n", current_input);
  fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}


// 新しいトークンを作成してcurに繋げる
static Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

static bool startswith(char *p, char *q) {
  return memcmp(p,q,strlen(q)) == 0;
}

static bool is_alpha(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static bool is_alnum(char c) {
  return is_alpha(c) || ( '0' <= c && c <= '9');
}

static bool equal(Token *tok, char *op) {
  return strlen(op) == tok->len && !strncmp(tok->str, op, tok->len);
}


static bool is_keyword(Token *tok) {
  static char *kw[] = {
    "return",
    "if",
    "else",
    "for",
    "while",
  };
  for(int i=0; i<sizeof(kw) / sizeof(*kw); i++)
    if(equal(tok, kw[i]))
      return true;
  return false;
}

static void convert_keywords(Token *tok) {
  for(Token *t = tok; t->kind != TK_EOF; t = t->next)
    if(t->kind == TK_IDENT && is_keyword(t))
      t->kind = TK_RESERVED;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p) {
  current_input = p;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (startswith(p, "==") || startswith(p, "!=") ||
        startswith(p, "<=") || startswith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }
    if (strchr("+-*/()<>;={}", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    // Identifier
    if (is_alpha(*p)) {
      char *q = p;
      while(is_alnum(*p))
        p++;
      cur = new_token(TK_IDENT, cur, q, p-q);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);
  convert_keywords(head.next);
  return head.next;
}

