#include "compiler_return_statement_strategy.h"

bool CompilerReturnStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::return_stmt, ctx.traps_checked);
}
