#include "compiler_preset_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerPresetStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_pset(false);
  return context->compiled;
}
