#include "compiler_noop_statement_strategy.h"

bool NoopCompilerStatementStrategy::execute(CompilerStatementContext& ctx) {
  // Keep legacy behavior: comments return early and do not run trap checks.
  ctx.skip_post_trap_check = true;
  return true;
}
