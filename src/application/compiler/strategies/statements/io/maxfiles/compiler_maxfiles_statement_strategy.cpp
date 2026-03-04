#include "compiler_maxfiles_statement_strategy.h"

bool CompilerMaxfilesStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::maxfiles_stmt, ctx.traps_checked);
}
