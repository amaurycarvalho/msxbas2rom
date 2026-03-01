#include "screen_statement_strategy.h"

#include "generic_statement_strategy.h"
#include "parser.h"

bool ScreenStatementStrategy::parseScreenCopy(Parser& parser,
                                              LexerLine* statement) {
  Lexeme* next_lexeme = statement->getCurrentLexeme();
  LexerLine parm;
  int state = 0;
  bool result = false;

  parser.pushActionFromLexemeNode(next_lexeme);
  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = parser.coalesceLexeme(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("TO")) {
          state = 1;
          continue;
        } else {
          return false;
        }
      } break;

      case 1: {
        if (next_lexeme->isKeyword("SCROLL")) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!parser.evalExpressionTokens(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            return false;
          }
          state = 2;
          continue;
        }
      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!parser.evalExpressionTokens(&parm)) {
      return false;
    }

    result = true;
  }

  parser.popActionNodeRoot();
  return result;
}

bool ScreenStatementStrategy::parseScreenPaste(Parser& parser,
                                               LexerLine* statement) {
  Lexeme* next_lexeme = statement->getCurrentLexeme();
  bool result = false;

  parser.pushActionFromLexemeNode(next_lexeme);

  if ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->isKeyword("FROM")) {
      GenericStatementStrategy genericStrategy;
      result = genericStrategy.parseStatement(parser, statement);
    }
  }

  parser.popActionNodeRoot();
  return result;
}

bool ScreenStatementStrategy::parseScreenScroll(Parser& parser,
                                                LexerLine* statement) {
  Lexeme* next_lexeme = statement->getCurrentLexeme();
  bool result;

  parser.pushActionFromLexemeNode(next_lexeme);
  GenericStatementStrategy genericStrategy;
  result = genericStrategy.parseStatement(parser, statement);
  parser.popActionNodeRoot();

  return result;
}

bool ScreenStatementStrategy::parseScreenLoad(Parser& parser,
                                              LexerLine* statement) {
  Lexeme* next_lexeme = statement->getCurrentLexeme();
  bool result;

  parser.pushActionFromLexemeNode(next_lexeme);
  GenericStatementStrategy genericStrategy;
  result = genericStrategy.parseStatement(parser, statement);
  parser.popActionNodeRoot();

  return result;
}

bool ScreenStatementStrategy::parseScreenOn(Parser& parser,
                                            LexerLine* statement) {
  Lexeme* next_lexeme = statement->getCurrentLexeme();
  parser.pushActionFromLexemeNode(next_lexeme);
  parser.popActionNodeRoot();
  return true;
}

bool ScreenStatementStrategy::parseScreenOff(Parser& parser,
                                             LexerLine* statement) {
  return parseScreenOn(parser, statement);
}

bool ScreenStatementStrategy::parseStatement(Parser& parser,
                                             LexerLine* statement) {
  Lexeme* next_lexeme;

  if ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "COPY") {
        return parseScreenCopy(parser, statement);
      } else if (next_lexeme->value == "PASTE") {
        return parseScreenPaste(parser, statement);
      } else if (next_lexeme->value == "SCROLL") {
        return parseScreenScroll(parser, statement);
      } else if (next_lexeme->value == "LOAD") {
        return parseScreenLoad(parser, statement);
      } else if (next_lexeme->value == "ON") {
        return parseScreenOn(parser, statement);
      } else if (next_lexeme->value == "OFF") {
        return parseScreenOff(parser, statement);
      }
    }

    statement->getPreviousLexeme();
    GenericStatementStrategy genericStrategy;
    return genericStrategy.parseStatement(parser, statement);
  }

  return false;
}

bool ScreenStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                      Lexeme* lexeme) {
  (void)lexeme;
  return parseStatement(parser, statement);
}
