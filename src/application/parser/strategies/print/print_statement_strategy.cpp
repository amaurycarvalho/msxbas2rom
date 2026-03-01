#include "print_statement_strategy.h"

#include "lexer.h"
#include "parser.h"

bool PrintStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                     Lexeme* lexeme) {
  if (lexeme->value == "?") {
    lexeme->value = "PRINT";
    lexeme->name = lexeme->value;
  }

  return parser.evalCmdPrint(statement);
}
