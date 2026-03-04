#include "compiler_draw_statement_strategy.h"

bool CompilerDrawStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::draw_stmt, ctx.traps_checked);
}
