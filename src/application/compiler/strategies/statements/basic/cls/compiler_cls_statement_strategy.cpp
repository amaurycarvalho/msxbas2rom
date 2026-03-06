#include "compiler_cls_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerClsStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_cls();
  return context->compiled;
}
