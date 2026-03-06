#include "compiler_text_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerTextStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_text();
  return context->compiled;
}
