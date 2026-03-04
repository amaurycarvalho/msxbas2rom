#include "compiler_vpoke_statement_strategy.h"

bool CompilerVpokeStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::vpoke_stmt, ctx.traps_checked);
}
