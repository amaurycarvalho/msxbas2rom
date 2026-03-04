#include "compiler_close_statement_strategy.h"

bool CompilerCloseStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::close_stmt, ctx.traps_checked);
}
