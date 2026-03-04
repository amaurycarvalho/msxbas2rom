#include "compiler_width_statement_strategy.h"

bool CompilerWidthStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::width_stmt, ctx.traps_checked);
}
