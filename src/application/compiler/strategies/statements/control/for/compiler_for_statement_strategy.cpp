#include "compiler_for_statement_strategy.h"

bool CompilerForStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::for_stmt, ctx.traps_checked);
}
