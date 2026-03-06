#include "compiler_return_statement_strategy.h"

#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerReturnStatementStrategy::execute(CompilerContext* context) {
  context->traps_checked = context->codeHelper->addCheckTraps();
  context->stmtEmitter->cmd_return();
  return context->compiled;
}
