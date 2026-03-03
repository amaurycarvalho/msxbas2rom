#include "dim_statement_strategy.h"

#include "generic_statement_strategy.h"
#include "parser.h"

bool DimStatementStrategy::parseStatement(ParserContext& context, LexerLine* statement) {
  ActionNode *action = context.actionRoot, *subaction;
  Lexeme* lexeme;
  unsigned int i, t;

  GenericStatementStrategy genericStrategy;
  if (!genericStrategy.parseStatement(context, statement)) return false;

  t = action->actions.size();
  if (!t) return false;

  for (i = 0; i < t; i++) {
    subaction = action->actions[i];
    lexeme = subaction->lexeme;
    lexeme->isArray = true;
    lexeme->parm_count = subaction->actions.size();
    if (!lexeme->parm_count) {
      context.error_message = "Invalid array declaration: DIM size parameter is missing";
      return false;
    }
  }

  return true;
}

bool DimStatementStrategy::execute(ParserContext& context, LexerLine* statement, Lexeme* lexeme) {
  (void)lexeme;
  return parseStatement(context, statement);
}
