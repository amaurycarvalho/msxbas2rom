#include "compiler_print_statement_strategy.h"

bool CompilerPrintStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::print_stmt, ctx.traps_checked);
}
