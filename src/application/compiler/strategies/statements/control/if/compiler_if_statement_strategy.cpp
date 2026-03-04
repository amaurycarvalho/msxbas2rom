#include "compiler_if_statement_strategy.h"

bool CompilerIfStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::if_stmt, ctx.traps_checked);
}
