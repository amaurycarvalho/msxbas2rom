#include "compiler_preset_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_pset_statement_strategy.h"

bool CompilerPresetStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_pset(context, false);
  return context->compiled;
}
