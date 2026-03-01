#include "get_statement_strategy.h"

#include "generic_statement_strategy.h"
#include "parser.h"

bool GetStatementStrategy::parseGetTile(Parser& parser, LexerLine* statement) {
  return parseGetSprite(parser, statement);
}

bool GetStatementStrategy::parseGetSprite(Parser& parser,
                                          LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  bool result = false;

  if ((next_lexeme = statement->getNextLexeme())) {
    parser.coalesceLexeme(next_lexeme);

    next_lexeme = statement->getCurrentLexeme();
    action = new ActionNode(next_lexeme);
    parser.pushActionNodeRoot(action);

    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "COLOR" || next_lexeme->value == "PATTERN") {
        result = parser.evalCmdSetSpriteColpattra(statement);
      }
    }

    parser.popActionNodeRoot();
  }

  return result;
}

bool GetStatementStrategy::parseStatement(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  bool result = false;

  if ((next_lexeme = statement->getNextLexeme())) {
    parser.coalesceLexeme(next_lexeme);

    next_lexeme = statement->getCurrentLexeme();
    action = new ActionNode(next_lexeme);
    parser.pushActionNodeRoot(action);

    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "DATE" || next_lexeme->value == "TIME") {
        GenericStatementStrategy genericStrategy;
        result = genericStrategy.parseStatement(parser, statement);
      } else if (next_lexeme->value == "TILE") {
        result = parseGetTile(parser, statement);
      } else if (next_lexeme->value == "SPRITE") {
        result = parseGetSprite(parser, statement);
      }
    }

    parser.popActionNodeRoot();
  }

  return result;
}

bool GetStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                   Lexeme* lexeme) {
  (void)lexeme;
  return parseStatement(parser, statement);
}
