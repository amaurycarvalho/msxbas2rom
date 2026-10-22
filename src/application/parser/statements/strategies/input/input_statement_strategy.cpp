#include "input_statement_strategy.h"

#include "print_statement_strategy.h"

bool InputStatementStrategy::parseStatement(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  PrintStatementStrategy printStrategy;
  return printStrategy.parseStatement(context, statement);
}

bool InputStatementStrategy::execute(shared_ptr<ParserContext> context,
                                     shared_ptr<LexerLineContext> statement,
                                     shared_ptr<Lexeme> lexeme) {
  (void)lexeme;
  context->getHasInput() = true;

  return parseStatement(context, statement);
}
