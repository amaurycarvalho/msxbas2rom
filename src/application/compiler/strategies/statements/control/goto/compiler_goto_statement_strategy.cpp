#include "compiler_goto_statement_strategy.h"

bool CompilerGotoStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::goto_stmt, ctx.traps_checked);
}
