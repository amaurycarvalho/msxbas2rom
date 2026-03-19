#include "compiler_redim_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_dim_statement_strategy.h"

bool CompilerRedimStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  CompilerDimStatementStrategy dim;
  return dim.execute(context);
}
