#include "compiler_noop_function_strategy.h"

#include "compiler_context.h"
#include "lexeme.h"

int NoopCompilerFunctionStrategy::execute(CompilerContext* context,
                                          ActionNode* action,
                                          int* result,
                                          unsigned int parmCount) {
  (void)context;
  (void)action;
  (void)result;
  (void)parmCount;
  return Lexeme::subtype_unknown;
}
