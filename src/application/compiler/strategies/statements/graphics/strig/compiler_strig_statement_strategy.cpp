#include "compiler_strig_statement_strategy.h"

bool CompilerStrigStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::strig_stmt, ctx.traps_checked);
}
