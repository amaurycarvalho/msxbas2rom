#include "time_statement_strategy.h"

#include "assignment_evaluator.h"
#include "lexeme.h"
#include "logger.h"

bool TimeStatementStrategy::parseStatement(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  return context->assignEval->evaluate(statement);
}

bool TimeStatementStrategy::execute(shared_ptr<ParserContext> context,
                                    shared_ptr<LexerLineContext> statement,
                                    shared_ptr<Lexeme> lexeme) {
  (void)lexeme;
  context->has_input = true;

  if (!context->assignEval) {
    context->logger->error(
        "TIME assignment internal error (parser context not initialized)");
    return false;
  }

  return parseStatement(context, statement);
}
