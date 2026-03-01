#include "call_statement_strategy.h"

#include "lexer.h"
#include "parser.h"

bool CallStatementStrategy::parseCall(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;

  while ((next_lexeme = statement->getNextLexeme())) {
    parser.pushActionFromLexemeNode(next_lexeme);
  }

  return true;
}

bool CallStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                    Lexeme* lexeme) {
  if (lexeme->value == "_") {
    lexeme->value = "CALL";
    lexeme->name = lexeme->value;
  }

  return parseCall(parser, statement);
}
