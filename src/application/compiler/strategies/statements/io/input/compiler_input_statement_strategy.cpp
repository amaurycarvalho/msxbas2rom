#include "compiler_input_statement_strategy.h"

bool CompilerInputStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::input_stmt, ctx.traps_checked);
}
