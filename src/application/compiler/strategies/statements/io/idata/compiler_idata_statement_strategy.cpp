#include "compiler_idata_statement_strategy.h"

#include "compiler_context.h"

void CompilerIdataStatementStrategy::cmd_idata(CompilerContext* context) {
  if (!context->current_action->actions.size()) {
    context->syntaxError("IDATA with empty parameters");
  }
}

bool CompilerIdataStatementStrategy::execute(CompilerContext* context) {
  cmd_idata(context);
  return context->compiled;
}
