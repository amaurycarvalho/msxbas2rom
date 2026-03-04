#include "compiler_dim_statement_strategy.h"

bool CompilerDimStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::dim_stmt, ctx.traps_checked);
}
