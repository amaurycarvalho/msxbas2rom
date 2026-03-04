#include "compiler_defint_statement_strategy.h"

bool CompilerDefintStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::def_stmt, ctx.traps_checked);
}
