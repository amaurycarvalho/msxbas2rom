#include "compiler_iread_statement_strategy.h"

bool CompilerIreadStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::iread_stmt, ctx.traps_checked);
}
