#include "compiler_cmd_statement_strategy.h"

bool CompilerCmdStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::cmd_stmt, ctx.traps_checked);
}
