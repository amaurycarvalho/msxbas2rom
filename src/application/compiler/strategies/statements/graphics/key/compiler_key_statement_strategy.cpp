#include "compiler_key_statement_strategy.h"

bool CompilerKeyStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::key_stmt, ctx.traps_checked);
}
