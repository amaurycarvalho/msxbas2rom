#include "compiler_get_statement_strategy.h"

bool CompilerGetStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::get_stmt, ctx.traps_checked);
}
