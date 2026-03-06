#include "compiler_resume_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerResumeStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_resume();
  return context->compiled;
}
