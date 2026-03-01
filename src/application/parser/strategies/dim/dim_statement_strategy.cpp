#include "dim_statement_strategy.h"

#include "generic_statement_strategy.h"
#include "parser.h"

bool DimStatementStrategy::parseStatement(Parser& parser, LexerLine* statement) {
  ParserContext& ctx = parser.getContext();
  ActionNode *action = ctx.actionRoot, *subaction;
  Lexeme* lexeme;
  unsigned int i, t;

  GenericStatementStrategy genericStrategy;
  if (!genericStrategy.parseStatement(parser, statement)) return false;

  t = action->actions.size();
  if (!t) return false;

  for (i = 0; i < t; i++) {
    subaction = action->actions[i];
    lexeme = subaction->lexeme;
    lexeme->isArray = true;
    lexeme->parm_count = subaction->actions.size();
    if (!lexeme->parm_count) {
      ctx.error_message = "Invalid array declaration: DIM size parameter is missing";
      return false;
    }
  }

  return true;
}

bool DimStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                   Lexeme* lexeme) {
  (void)lexeme;
  return parseStatement(parser, statement);
}
