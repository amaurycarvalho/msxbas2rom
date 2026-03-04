#include "compiler_idata_statement_strategy.h"

bool CompilerIdataStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::idata_stmt, ctx.traps_checked);
}
