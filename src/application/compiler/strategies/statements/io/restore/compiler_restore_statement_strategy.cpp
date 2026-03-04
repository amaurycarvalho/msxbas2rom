#include "compiler_restore_statement_strategy.h"

bool CompilerRestoreStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::restore_stmt, ctx.traps_checked);
}
