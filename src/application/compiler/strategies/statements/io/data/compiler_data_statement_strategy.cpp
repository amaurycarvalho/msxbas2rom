#include "compiler_data_statement_strategy.h"

#include "compiler_context.h"

void CompilerDataStatementStrategy::cmd_data(CompilerContext* context) {
  if (!context->current_action->actions.size()) {
    context->syntaxError("DATA with empty parameters");
  }
}

bool CompilerDataStatementStrategy::execute(CompilerContext* context) {
  cmd_data(context);
  return context->compiled;
}
