#include "compiler_data_statement_strategy.h"

bool CompilerDataStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::data_stmt, ctx.traps_checked);
}
