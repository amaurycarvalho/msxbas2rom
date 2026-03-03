#include "let_statement_strategy.h"

#include "assignment_evaluator.h"
#include "expression_evaluator.h"

bool LetStatementStrategy::parseStatement(ParserContext& context,
                                          LexerLine* statement) {
  ExpressionEvaluator exprEval(context);
  AssignmentEvaluator assignEval(context, exprEval);
  return assignEval.evaluate(statement);
}

bool LetStatementStrategy::execute(ParserContext& context, LexerLine* statement, Lexeme* lexeme) {
  (void)lexeme;
  return parseStatement(context, statement);
}
