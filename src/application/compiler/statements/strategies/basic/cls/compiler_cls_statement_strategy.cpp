#include "compiler_cls_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_hooks.h"

void CompilerClsStatementStrategy::cmd_cls(
    shared_ptr<CompilerContext> context) {
  context->codeOptimizer->addKernelCall(DISP_XBASIC_CLS);  // call cls
}

bool CompilerClsStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_cls(context);
  return context->compiled;
}
