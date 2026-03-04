#include "compiler_irestore_statement_strategy.h"

bool CompilerIrestoreStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::irestore_stmt, ctx.traps_checked);
}
