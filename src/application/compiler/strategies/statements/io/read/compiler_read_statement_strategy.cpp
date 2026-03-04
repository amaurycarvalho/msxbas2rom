#include "compiler_read_statement_strategy.h"

bool CompilerReadStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::read_stmt, ctx.traps_checked);
}
