#include "compiler_text_statement_strategy.h"

bool CompilerTextStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::text_stmt, ctx.traps_checked);
}
