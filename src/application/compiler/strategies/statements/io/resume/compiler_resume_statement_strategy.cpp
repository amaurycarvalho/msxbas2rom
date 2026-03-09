#include "compiler_resume_statement_strategy.h"

#include "compiler_context.h"

void CompilerResumeStatementStrategy::cmd_resume(CompilerContext* context) {
  context->syntaxError("RESUME statement not supported in compiled mode");
}

bool CompilerResumeStatementStrategy::execute(CompilerContext* context) {
  cmd_resume(context);
  return context->compiled;
}
