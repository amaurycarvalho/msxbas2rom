#include "compiler_time_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_let_statement_strategy.h"

void CompilerTimeStatementStrategy::cmd_time(CompilerContext* context) {
  CompilerLetStatementStrategy let;
  let.execute(context);
}

bool CompilerTimeStatementStrategy::execute(CompilerContext* context) {
  cmd_time(context);
  return context->compiled;
}
