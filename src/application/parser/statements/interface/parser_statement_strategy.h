#ifndef PARSER_STATEMENT_STRATEGY_H_INCLUDED
#define PARSER_STATEMENT_STRATEGY_H_INCLUDED

#include "expression_evaluator.h"
#include "parser_context.h"

inline bool evaluateExpression(shared_ptr<ParserContext> ctx,
                               shared_ptr<LexerLineContext> expression) {
  ExpressionEvaluator exprEval(ctx);
  return exprEval.evaluate(expression);
}

class IParserStatementStrategy {
 public:
  virtual ~IParserStatementStrategy() {}
  virtual bool execute(shared_ptr<ParserContext> ctx,
                       shared_ptr<LexerLineContext> statement,
                       shared_ptr<Lexeme> lexeme) = 0;
};

#endif  // PARSER_STATEMENT_STRATEGY_H_INCLUDED
