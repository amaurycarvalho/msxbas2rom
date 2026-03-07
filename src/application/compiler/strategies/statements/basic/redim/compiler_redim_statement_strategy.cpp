#include "compiler_redim_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_dim_statement_strategy.h"

void CompilerRedimStatementStrategy::cmd_redim(CompilerContext* context) {
  CompilerDimStatementStrategy dim;
  dim.execute(context);
}

bool CompilerRedimStatementStrategy::execute(CompilerContext* context) {
  cmd_redim(context);
  return context->compiled;
}
