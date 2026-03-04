#include "compiler_next_statement_strategy.h"

bool CompilerNextStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::next_stmt, ctx.traps_checked);
}
