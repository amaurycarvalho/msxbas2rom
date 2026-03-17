#include "get_statement_strategy.h"

#include "generic_statement_strategy.h"
#include "set_statement_strategy.h"

bool GetStatementStrategy::parseGetTile(ParserContext& context,
                                        LexerLineContext* statement) {
  return parseGetSprite(context, statement);
}

bool GetStatementStrategy::parseGetSprite(ParserContext& context,
                                          LexerLineContext* statement) {
  shared_ptr<Lexeme> next_lexeme;
  shared_ptr<ActionNode> action;
  bool result = false;

  if ((next_lexeme = statement->getNextLexeme())) {
    context.coalesceSymbols(next_lexeme);

    next_lexeme = statement->getCurrentLexeme();
    action = make_shared<ActionNode>(next_lexeme);
    context.pushActionRoot(action);

    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "COLOR" || next_lexeme->value == "PATTERN") {
        SetStatementStrategy strategy;
        return strategy.parseSetSpriteColpattra(context, statement);
      }
    }

    context.popActionRoot();
  }

  return result;
}

bool GetStatementStrategy::parseStatement(ParserContext& context,
                                          LexerLineContext* statement) {
  shared_ptr<Lexeme> next_lexeme;
  shared_ptr<ActionNode> action;
  bool result = false;

  if ((next_lexeme = statement->getNextLexeme())) {
    context.coalesceSymbols(next_lexeme);

    next_lexeme = statement->getCurrentLexeme();
    action = make_shared<ActionNode>(next_lexeme);
    context.pushActionRoot(action);

    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "DATE" || next_lexeme->value == "TIME") {
        GenericStatementStrategy genericStrategy;
        result = genericStrategy.parseStatement(context, statement);
      } else if (next_lexeme->value == "TILE") {
        result = parseGetTile(context, statement);
      } else if (next_lexeme->value == "SPRITE") {
        result = parseGetSprite(context, statement);
      }
    }

    context.popActionRoot();
  }

  return result;
}

bool GetStatementStrategy::execute(ParserContext& context,
                                   LexerLineContext* statement,
                                   shared_ptr<Lexeme> lexeme) {
  (void)lexeme;
  return parseStatement(context, statement);
}
