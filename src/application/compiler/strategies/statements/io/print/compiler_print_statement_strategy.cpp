#include "compiler_print_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerPrintStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_print();
  return context->compiled;
}
