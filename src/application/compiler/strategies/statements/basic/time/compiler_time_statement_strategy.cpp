#include "compiler_time_statement_strategy.h"

bool CompilerTimeStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::let_stmt, ctx.traps_checked);
}
