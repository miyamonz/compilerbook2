#include "9cc.h"

static int align_to(int n, int align) {
  return (n + align - 1) & ~(align - 1);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません");
    return 1;
  }

  // トークナイズしてパースする
  token = tokenize(argv[1]);
  Function *prog = parse();

  int offset = 0;
  for(Var *var = prog->locals; var; var = var->next) {
    offset += 8;
    var->offset = offset;
  }
  //shoud align
  prog->stack_size = align_to(offset, 16);
  codegen(prog);

  return 0;
}
