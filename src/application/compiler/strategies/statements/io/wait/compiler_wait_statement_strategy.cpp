#include "compiler_wait_statement_strategy.h"

bool CompilerWaitStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::wait_stmt, ctx.traps_checked);
}
