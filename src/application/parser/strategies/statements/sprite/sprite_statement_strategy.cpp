#include "sprite_statement_strategy.h"

#include "generic_statement_strategy.h"
#include "parser.h"

bool SpriteStatementStrategy::parseSpriteLoad(ParserContext& context, LexerLine* statement) {
  Lexeme* next_lexeme = statement->getCurrentLexeme();
  bool result;

  context.pushActionFromLexeme(next_lexeme);
  GenericStatementStrategy genericStrategy;
  result = genericStrategy.parseStatement(context, statement);
  context.popActionRoot();

  return result;
}

bool SpriteStatementStrategy::parseStatement(ParserContext& context, LexerLine* statement) {
  Lexeme* next_lexeme;

  if ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "ON" || next_lexeme->value == "OFF" ||
          next_lexeme->value == "STOP") {
        context.pushActionFromLexeme(next_lexeme);
        return true;

      } else if (next_lexeme->value == "LOAD") {
        return parseSpriteLoad(context, statement);
      }
    }
  }

  return false;
}

bool SpriteStatementStrategy::execute(ParserContext& context, LexerLine* statement, Lexeme* lexeme) {
  (void)lexeme;
  return parseStatement(context, statement);
}
