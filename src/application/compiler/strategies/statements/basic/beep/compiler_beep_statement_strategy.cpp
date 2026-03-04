#include "compiler_beep_statement_strategy.h"

bool CompilerBeepStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::beep_stmt, ctx.traps_checked);
}
