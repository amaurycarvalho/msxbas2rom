#include "input_statement_strategy.h"

#include "print_statement_strategy.h"

bool InputStatementStrategy::parseStatement(ParserContext& context,
                                            LexerLineContext* statement) {
  PrintStatementStrategy printStrategy;
  return printStrategy.parseStatement(context, statement);
}

bool InputStatementStrategy::execute(ParserContext& context,
                                     LexerLineContext* statement,
                                     shared_ptr<Lexeme> lexeme) {
  (void)lexeme;
  context.has_input = true;

  return parseStatement(context, statement);
}
