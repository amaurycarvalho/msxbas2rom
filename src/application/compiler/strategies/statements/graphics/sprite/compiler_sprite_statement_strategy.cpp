#include "compiler_sprite_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerSpriteStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_sprite();
  return context->compiled;
}
