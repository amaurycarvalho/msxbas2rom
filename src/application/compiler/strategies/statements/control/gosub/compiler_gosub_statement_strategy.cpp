#include "compiler_gosub_statement_strategy.h"

bool CompilerGosubStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::gosub_stmt, ctx.traps_checked);
}
