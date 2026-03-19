#include "call_statement_strategy.h"

#include "lexeme.h"
#include "lexer_line_context.h"

bool CallStatementStrategy::parseCall(shared_ptr<ParserContext> context,
                                      shared_ptr<LexerLineContext> statement) {
  (void)context;
  shared_ptr<Lexeme> next_lexeme;

  while ((next_lexeme = statement->getNextLexeme())) {
    context->pushActionFromLexeme(next_lexeme);
  }

  return true;
}

bool CallStatementStrategy::execute(shared_ptr<ParserContext> context,
                                    shared_ptr<LexerLineContext> statement,
                                    shared_ptr<Lexeme> lexeme) {
  if (lexeme->value == "_") {
    lexeme->value = "CALL";
    lexeme->name = lexeme->value;
  }

  return parseCall(context, statement);
}
