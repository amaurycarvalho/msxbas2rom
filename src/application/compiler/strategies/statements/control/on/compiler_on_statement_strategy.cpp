#include "compiler_on_statement_strategy.h"

bool CompilerOnStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::on_stmt, ctx.traps_checked);
}
