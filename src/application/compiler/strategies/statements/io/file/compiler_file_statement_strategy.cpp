#include "compiler_file_statement_strategy.h"

bool CompilerFileStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::file_stmt, ctx.traps_checked);
}
