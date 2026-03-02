#include "time_statement_strategy.h"

#include "let_statement_strategy.h"
#include "parser.h"

bool TimeStatementStrategy::parseStatement(Parser& parser,
                                           LexerLine* statement) {
  LetStatementStrategy letStrategy;
  return letStrategy.parseStatement(parser, statement);
}

bool TimeStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                    Lexeme* lexeme) {
  (void)lexeme;

  ParserContext& ctx = parser.getContext();
  ctx.has_input = true;

  return parseStatement(parser, statement);
}
