#include "compiler_play_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerPlayStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_play();
  return context->compiled;
}
