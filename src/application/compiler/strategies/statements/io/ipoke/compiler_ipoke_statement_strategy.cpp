#include "compiler_ipoke_statement_strategy.h"

bool CompilerIpokeStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::ipoke_stmt, ctx.traps_checked);
}
