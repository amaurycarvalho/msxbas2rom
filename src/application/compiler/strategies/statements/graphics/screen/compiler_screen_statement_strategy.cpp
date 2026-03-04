#include "compiler_screen_statement_strategy.h"

bool CompilerScreenStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::screen_stmt, ctx.traps_checked);
}
