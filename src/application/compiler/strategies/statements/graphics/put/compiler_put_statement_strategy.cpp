#include "compiler_put_statement_strategy.h"

bool CompilerPutStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::put_stmt, ctx.traps_checked);
}
