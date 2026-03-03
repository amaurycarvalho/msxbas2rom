#include "noop_statement_strategy.h"

#include "lexer.h"

bool NoopStatementStrategy::execute(ParserContext& context, LexerLine* statement, Lexeme* lexeme) {
  (void)context;
  (void)statement;

  if (lexeme->value == "'") {
    lexeme->type = Lexeme::type_keyword;
    lexeme->name = "REM";
    lexeme->value = lexeme->name;
  }

  return true;
}
