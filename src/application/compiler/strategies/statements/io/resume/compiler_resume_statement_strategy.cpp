#include "compiler_resume_statement_strategy.h"

bool CompilerResumeStatementStrategy::execute(CompilerStatementContext& ctx) {
  return ctx.dispatch(CompilerCommandId::resume_stmt, ctx.traps_checked);
}
