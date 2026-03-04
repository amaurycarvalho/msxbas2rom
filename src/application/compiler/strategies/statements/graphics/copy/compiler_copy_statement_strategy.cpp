#include "compiler_copy_statement_strategy.h"

bool CompilerCopyStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::copy_stmt, ctx.traps_checked);
}
