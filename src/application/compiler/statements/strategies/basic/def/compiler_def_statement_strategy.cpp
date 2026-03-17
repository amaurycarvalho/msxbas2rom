#include "compiler_def_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_defusr_statement_strategy.h"

void CompilerDefStatementStrategy::cmd_def(CompilerContext* context) {
  ActionNode* action;
  shared_ptr<Lexeme> lexeme;
  unsigned int t = context->current_action->actions.size();

  if (t) {
    action = context->current_action->actions[0];
    lexeme = action->lexeme;

    if (lexeme->value == "USR") {
      CompilerDefUsrStatementStrategy defusr;
      defusr.execute(context);
    }
  }
}

bool CompilerDefStatementStrategy::execute(CompilerContext* context) {
  cmd_def(context);
  return context->compiled;
}
