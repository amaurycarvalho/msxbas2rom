#include "compiler_paint_statement_strategy.h"

bool CompilerPaintStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::paint_stmt, ctx.traps_checked);
}
