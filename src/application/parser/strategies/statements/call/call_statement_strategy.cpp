#include "call_statement_strategy.h"

#include "lexer.h"

bool CallStatementStrategy::parseCall(ParserContext& context,
                                      LexerLine* statement) {
  (void)context;
  Lexeme* next_lexeme;

  while ((next_lexeme = statement->getNextLexeme())) {
    context.pushActionFromLexeme(next_lexeme);
  }

  return true;
}

bool CallStatementStrategy::execute(ParserContext& context, LexerLine* statement, Lexeme* lexeme) {
  if (lexeme->value == "_") {
    lexeme->value = "CALL";
    lexeme->name = lexeme->value;
  }

  return parseCall(context, statement);
}
