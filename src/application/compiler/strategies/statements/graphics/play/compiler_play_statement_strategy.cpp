#include "compiler_play_statement_strategy.h"

bool CompilerPlayStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::play_stmt, ctx.traps_checked);
}
