#include "compiler_circle_statement_strategy.h"

bool CompilerCircleStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::circle_stmt, ctx.traps_checked);
}
