#include "compiler_poke_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerPokeStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_poke();
  return context->compiled;
}
