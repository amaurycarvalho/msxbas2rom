#include "sprite_statement_strategy.h"

#include "generic_statement_strategy.h"
#include "parser.h"

bool SpriteStatementStrategy::parseSpriteLoad(Parser& parser,
                                              LexerLine* statement) {
  Lexeme* next_lexeme = statement->getCurrentLexeme();
  bool result;

  parser.pushActionFromLexemeNode(next_lexeme);
  GenericStatementStrategy genericStrategy;
  result = genericStrategy.parseStatement(parser, statement);
  parser.popActionNodeRoot();

  return result;
}

bool SpriteStatementStrategy::parseStatement(Parser& parser,
                                             LexerLine* statement) {
  Lexeme* next_lexeme;

  if ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "ON" || next_lexeme->value == "OFF" ||
          next_lexeme->value == "STOP") {
        parser.pushActionFromLexemeNode(next_lexeme);
        return true;

      } else if (next_lexeme->value == "LOAD") {
        return parseSpriteLoad(parser, statement);
      }
    }
  }

  return false;
}

bool SpriteStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                      Lexeme* lexeme) {
  (void)lexeme;
  return parseStatement(parser, statement);
}
