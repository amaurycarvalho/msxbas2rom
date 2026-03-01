#include "let_statement_strategy.h"

#include "parser.h"

bool LetStatementStrategy::parseStatement(Parser& parser, LexerLine* statement) {
  return parser.evalAssignmentTokens(statement);
}

bool LetStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                   Lexeme* lexeme) {
  (void)lexeme;
  return parseStatement(parser, statement);
}
