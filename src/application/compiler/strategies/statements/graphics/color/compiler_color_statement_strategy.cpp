#include "compiler_color_statement_strategy.h"

bool CompilerColorStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::color_stmt, ctx.traps_checked);
}
