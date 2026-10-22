#include "compiler_noop_statement_strategy.h"

#include "compiler_context.h"

bool NoopCompilerStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  // Keep legacy behavior: comments return early and do not run trap checks.
  context->getSkipPostTrapCheck() = true;
  return context->getCompiled();
}
