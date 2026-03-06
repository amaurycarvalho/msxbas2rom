#include "compiler_file_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_statement_emitter.h"

bool CompilerFileStatementStrategy::execute(CompilerContext* context) {
  context->stmtEmitter->cmd_file();
  return context->compiled;
}
