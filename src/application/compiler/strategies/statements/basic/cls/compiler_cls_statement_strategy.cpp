#include "compiler_cls_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_hooks.h"

void CompilerClsStatementStrategy::cmd_cls(CompilerContext* context) {
  auto& cpu = *context->cpu;
  cpu.addCall(def_XBASIC_CLS);  // call cls
}

bool CompilerClsStatementStrategy::execute(CompilerContext* context) {
  cmd_cls(context);
  return context->compiled;
}
