#include "let_statement_strategy.h"

#include "assignment_evaluator.h"
#include "lexeme.h"
#include "logger.h"

bool LetStatementStrategy::parseStatement(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  return context->assignEval->evaluate(statement);
}

bool LetStatementStrategy::execute(shared_ptr<ParserContext> context,
                                   shared_ptr<LexerLineContext> statement,
                                   shared_ptr<Lexeme> lexeme) {
  (void)lexeme;
  if (!context->assignEval) {
    context->logger->error(
        "LET statement internal error (parser context not initialized)");
    return false;
  }
  return parseStatement(context, statement);
}
