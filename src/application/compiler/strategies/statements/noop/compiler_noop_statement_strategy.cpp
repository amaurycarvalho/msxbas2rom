#include "compiler_noop_statement_strategy.h"

#include "compiler_context.h"

bool NoopCompilerStatementStrategy::execute(CompilerContext* context) {
  // Keep legacy behavior: comments return early and do not run trap checks.
  context->skip_post_trap_check = true;
  return context->compiled;
}
