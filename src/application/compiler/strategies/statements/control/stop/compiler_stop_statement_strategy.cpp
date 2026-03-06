#include "compiler_stop_statement_strategy.h"

#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerStopStatementStrategy::execute(CompilerContext* context) {
  context->traps_checked = context->codeHelper->addCheckTraps();
  context->stmtEmitter->cmd_stop();
  return context->compiled;
}
