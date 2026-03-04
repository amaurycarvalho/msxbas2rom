#include "compiler_set_statement_strategy.h"

bool CompilerSetStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::set_stmt, ctx.traps_checked);
}
