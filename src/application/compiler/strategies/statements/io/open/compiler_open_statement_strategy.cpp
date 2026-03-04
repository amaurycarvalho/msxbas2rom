#include "compiler_open_statement_strategy.h"

bool CompilerOpenStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::open_stmt, ctx.traps_checked);
}
