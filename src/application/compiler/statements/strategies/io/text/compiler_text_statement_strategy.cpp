#include "compiler_text_statement_strategy.h"

#include "compiler_context.h"
#include "resources.h"

void CompilerTextStatementStrategy::cmd_text(CompilerContext* context) {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int t = context->current_action->actions.size();

  if (t == 1) {
    action = context->current_action->actions[0];
    lexeme = action->lexeme;

    if (lexeme->type == Lexeme::type_literal &&
        lexeme->subtype == Lexeme::subtype_string) {
      lexeme->name = "TEXT";
      context->resourceManager->addText(lexeme->value);
    } else {
      context->syntaxError("Invalid parameter in TEXT keyword");
    }

  } else {
    context->syntaxError("Wrong TEXT parameters count");
  }
}

bool CompilerTextStatementStrategy::execute(CompilerContext* context) {
  cmd_text(context);
  return context->compiled;
}
