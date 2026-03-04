#include "compiler_stop_statement_strategy.h"

bool CompilerStopStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::stop_stmt, ctx.traps_checked);
}
