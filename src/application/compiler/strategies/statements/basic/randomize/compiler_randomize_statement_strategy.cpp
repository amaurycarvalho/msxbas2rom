#include "compiler_randomize_statement_strategy.h"

bool CompilerRandomizeStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::randomize_stmt, ctx.traps_checked);
}
