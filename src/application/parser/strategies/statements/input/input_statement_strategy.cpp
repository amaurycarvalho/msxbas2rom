#include "input_statement_strategy.h"

#include "print_statement_strategy.h"

bool InputStatementStrategy::parseStatement(ParserContext& context, LexerLine* statement) {
  PrintStatementStrategy printStrategy;
  return printStrategy.parseStatement(context, statement);
}

bool InputStatementStrategy::execute(ParserContext& context, LexerLine* statement, Lexeme* lexeme) {
  (void)lexeme;
  context.has_input = true;

  return parseStatement(context, statement);
}
