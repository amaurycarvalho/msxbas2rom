#include "compiler_data_statement_strategy.h"

#include "action_node.h"
#include "compiler_context.h"

void CompilerDataStatementStrategy::cmd_data(
    shared_ptr<CompilerContext> context) {
  if (!context->current_action->actions.size()) {
    context->syntaxError("DATA with empty parameters");
  }
}

bool CompilerDataStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_data(context);
  return context->compiled;
}
