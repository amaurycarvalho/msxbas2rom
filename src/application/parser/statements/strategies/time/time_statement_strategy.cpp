#include "time_statement_strategy.h"

#include "let_statement_strategy.h"

bool TimeStatementStrategy::parseStatement(ParserContext& context,
                                           LexerLineContext* statement) {
  LetStatementStrategy letStrategy;
  return letStrategy.parseStatement(context, statement);
}

bool TimeStatementStrategy::execute(ParserContext& context,
                                    LexerLineContext* statement,
                                    shared_ptr<Lexeme> lexeme) {
  (void)lexeme;
  context.has_input = true;

  return parseStatement(context, statement);
}
