#include "compiler_redim_statement_strategy.h"

bool CompilerRedimStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::redim_stmt, ctx.traps_checked);
}
