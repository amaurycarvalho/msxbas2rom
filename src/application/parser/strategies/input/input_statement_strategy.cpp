#include "input_statement_strategy.h"

#include "parser.h"

bool InputStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                     Lexeme* lexeme) {
  (void)lexeme;

  ParserContext& ctx = parser.getContext();
  ctx.has_input = true;

  return parser.evalCmdInput(statement);
}
