#include "compiler_defdbl_statement_strategy.h"

bool CompilerDefdblStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::def_stmt, ctx.traps_checked);
}
