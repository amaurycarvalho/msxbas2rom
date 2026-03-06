#include "compiler_open_grp_statement_strategy.h"

#include "compiler_code_helper.h"
#include "compiler_context.h"

bool CompilerOpenGrpStatementStrategy::execute(CompilerContext* context) {
  context->has_open_grp = true;
  return context->compiled;
}
