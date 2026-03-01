#include "next_statement_strategy.h"

#include "parser.h"

bool NextStatementStrategy::parseStatement(Parser& parser, LexerLine* statement) {
  Lexeme *next_lexeme, *current_lexeme = parser.getContext().actionRoot->lexeme;
  ActionNode* action;
  int sepCount = 0;

  while ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->isSeparator("(")) {
      sepCount++;
    } else if (next_lexeme->isSeparator(")")) {
      if (sepCount) sepCount--;
    } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
      parser.popActionNodeRoot();
      action = new ActionNode(current_lexeme);
      parser.pushActionNodeRoot(action);
    }
  }

  return true;
}

bool NextStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                    Lexeme* lexeme) {
  (void)lexeme;
  return parseStatement(parser, statement);
}
