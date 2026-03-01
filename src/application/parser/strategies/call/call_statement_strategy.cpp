#include "call_statement_strategy.h"

#include "lexer.h"
#include "parser.h"

bool CallStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                    Lexeme* lexeme) {
  if (lexeme->value == "_") {
    lexeme->value = "CALL";
    lexeme->name = lexeme->value;
  }

  return parser.evalCmdCall(statement);
}
