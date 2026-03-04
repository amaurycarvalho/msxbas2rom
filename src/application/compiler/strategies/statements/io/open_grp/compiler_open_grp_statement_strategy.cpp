#include "compiler_open_grp_statement_strategy.h"

bool CompilerOpenGrpStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::open_grp_stmt, ctx.traps_checked);
}
