#include "noop_statement_strategy.h"

#include "lexeme.h"

bool NoopStatementStrategy::execute(shared_ptr<ParserContext> context,
                                    shared_ptr<LexerLineContext> statement,
                                    shared_ptr<Lexeme> lexeme) {
  (void)context;
  (void)statement;

  if (lexeme->value == "'") {
    lexeme->type = Lexeme::type_keyword;
    lexeme->name = "REM";
    lexeme->value = lexeme->name;
  }

  return true;
}
