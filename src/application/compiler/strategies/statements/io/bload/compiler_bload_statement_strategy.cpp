#include "compiler_bload_statement_strategy.h"

bool CompilerBloadStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::bload_stmt, ctx.traps_checked);
}
