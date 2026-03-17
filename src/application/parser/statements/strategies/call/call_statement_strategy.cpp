#include "call_statement_strategy.h"

#include "lexer.h"

bool CallStatementStrategy::parseCall(ParserContext& context,
                                      LexerLineContext* statement) {
  (void)context;
  shared_ptr<Lexeme> next_lexeme;

  while ((next_lexeme = statement->getNextLexeme())) {
    context.pushActionFromLexeme(next_lexeme);
  }

  return true;
}

bool CallStatementStrategy::execute(ParserContext& context,
                                    LexerLineContext* statement,
                                    shared_ptr<Lexeme> lexeme) {
  if (lexeme->value == "_") {
    lexeme->value = "CALL";
    lexeme->name = lexeme->value;
  }

  return parseCall(context, statement);
}
