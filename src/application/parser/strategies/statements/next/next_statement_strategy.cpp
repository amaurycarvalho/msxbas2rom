#include "next_statement_strategy.h"

#include "parser.h"

bool NextStatementStrategy::parseStatement(ParserContext& context,
                                           LexerLine* statement) {
  Lexeme *next_lexeme, *current_lexeme = context.actionRoot->lexeme;
  ActionNode* action;
  int sepCount = 0;

  while ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->isSeparator("(")) {
      sepCount++;
    } else if (next_lexeme->isSeparator(")")) {
      if (sepCount) sepCount--;
    } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
      context.popActionRoot();
      action = new ActionNode(current_lexeme);
      context.pushActionRoot(action);
    }
  }

  return true;
}

bool NextStatementStrategy::execute(ParserContext& context, LexerLine* statement, Lexeme* lexeme) {
  (void)lexeme;
  return parseStatement(context, statement);
}
