#include "compiler_end_statement_strategy.h"

bool CompilerEndStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::end_stmt, ctx.traps_checked);
}
