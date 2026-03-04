#include "compiler_defstr_statement_strategy.h"

bool CompilerDefstrStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::def_stmt, ctx.traps_checked);
}
