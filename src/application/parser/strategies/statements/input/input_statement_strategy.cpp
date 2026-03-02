#include "input_statement_strategy.h"

#include "print_statement_strategy.h"
#include "parser.h"

bool InputStatementStrategy::parseStatement(Parser& parser, LexerLine* statement) {
  PrintStatementStrategy printStrategy;
  return printStrategy.parseStatement(parser, statement);
}

bool InputStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                     Lexeme* lexeme) {
  (void)lexeme;

  ParserContext& ctx = parser.getContext();
  ctx.has_input = true;

  return parseStatement(parser, statement);
}
