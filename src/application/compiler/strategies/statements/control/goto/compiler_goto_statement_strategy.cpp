#include "compiler_goto_statement_strategy.h"

#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerGotoStatementStrategy::execute(CompilerContext* context) {
  context->traps_checked = context->codeHelper->addCheckTraps();
  context->stmtEmitter->cmd_goto();
  return context->compiled;
}
