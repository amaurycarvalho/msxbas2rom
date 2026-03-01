#include "let_statement_strategy.h"

#include "parser.h"

bool LetStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                   Lexeme* lexeme) {
  (void)lexeme;
  return parser.evalCmdLet(statement);
}
