#include "compiler_defsng_statement_strategy.h"

bool CompilerDefsngStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::def_stmt, ctx.traps_checked);
}
