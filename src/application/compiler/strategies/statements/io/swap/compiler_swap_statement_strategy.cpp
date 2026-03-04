#include "compiler_swap_statement_strategy.h"

bool CompilerSwapStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::swap_stmt, ctx.traps_checked);
}
