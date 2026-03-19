#include "compiler_text_statement_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "lexeme.h"
#include "resources.h"

void CompilerTextStatementStrategy::cmd_text(
    shared_ptr<CompilerContext> context) {
  shared_ptr<Lexeme> lexeme;
  shared_ptr<ActionNode> action;
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

bool CompilerTextStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_text(context);
  return context->compiled;
}
