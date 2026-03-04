#include "compiler_clear_statement_strategy.h"

bool CompilerClearStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::clear_stmt, ctx.traps_checked);
}
