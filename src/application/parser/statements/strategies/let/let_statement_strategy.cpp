#include "let_statement_strategy.h"

#include "assignment_evaluator.h"
#include "expression_evaluator.h"

bool LetStatementStrategy::parseStatement(ParserContext& context,
                                          LexerLineContext* statement) {
  ExpressionEvaluator exprEval(context);
  AssignmentEvaluator assignEval(context, exprEval);
  return assignEval.evaluate(statement);
}

bool LetStatementStrategy::execute(ParserContext& context,
                                   LexerLineContext* statement,
                                   shared_ptr<Lexeme> lexeme) {
  (void)lexeme;
  return parseStatement(context, statement);
}
