#include "compiler_call_statement_strategy.h"

bool CompilerCallStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::call_stmt, ctx.traps_checked);
}
