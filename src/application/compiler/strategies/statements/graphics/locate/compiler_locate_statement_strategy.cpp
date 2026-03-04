#include "compiler_locate_statement_strategy.h"

bool CompilerLocateStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::locate_stmt, ctx.traps_checked);
}
