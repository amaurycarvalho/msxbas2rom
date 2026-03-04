#include "compiler_def_statement_strategy.h"

bool CompilerDefStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::def_stmt, ctx.traps_checked);
}
