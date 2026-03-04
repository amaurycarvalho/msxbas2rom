#include "compiler_interval_statement_strategy.h"

bool CompilerIntervalStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::interval_stmt, ctx.traps_checked);
}
