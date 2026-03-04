#include "compiler_out_statement_strategy.h"

bool CompilerOutStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::out_stmt, ctx.traps_checked);
}
