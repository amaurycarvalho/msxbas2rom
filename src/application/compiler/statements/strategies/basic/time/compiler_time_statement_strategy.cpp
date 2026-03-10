#include "compiler_time_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_let_statement_strategy.h"

bool CompilerTimeStatementStrategy::execute(CompilerContext* context) {
  CompilerLetStatementStrategy let;
  return let.execute(context);
}
