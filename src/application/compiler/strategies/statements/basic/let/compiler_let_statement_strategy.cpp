#include "compiler_let_statement_strategy.h"

bool CompilerLetStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::let_stmt, ctx.traps_checked);
}
