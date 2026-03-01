#include "noop_statement_strategy.h"

#include "lexer.h"
#include "parser.h"

bool NoopStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                    Lexeme* lexeme) {
  (void)parser;
  (void)statement;

  if (lexeme->value == "'") {
    lexeme->type = Lexeme::type_keyword;
    lexeme->name = "REM";
    lexeme->value = lexeme->name;
  }

  return true;
}
