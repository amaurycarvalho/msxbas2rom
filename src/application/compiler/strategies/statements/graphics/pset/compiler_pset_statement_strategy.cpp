#include "compiler_pset_statement_strategy.h"

bool CompilerPsetStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::pset_stmt, ctx.traps_checked);
}
