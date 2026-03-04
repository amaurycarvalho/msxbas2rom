#include "compiler_line_statement_strategy.h"

bool CompilerLineStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::line_stmt, ctx.traps_checked);
}
