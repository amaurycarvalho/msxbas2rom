#include "compiler_noop_function_strategy.h"

#include "compiler_context.h"

bool NoopCompilerFunctionStrategy::execute(CompilerContext* context) {
  // Keep legacy behavior: comments return early and do not run trap checks.
  // context->skip_post_trap_check = true;
  return context->compiled;
}
