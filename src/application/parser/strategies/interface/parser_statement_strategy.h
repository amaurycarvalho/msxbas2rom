#ifndef PARSER_STATEMENT_STRATEGY_H_INCLUDED
#define PARSER_STATEMENT_STRATEGY_H_INCLUDED

#include "expression_evaluator.h"
#include "parser_context.h"

inline bool evaluateExpression(ParserContext& ctx, LexerLine* expression) {
  ExpressionEvaluator exprEval(ctx);
  return exprEval.evaluate(expression);
}

class IParserStatementStrategy {
 public:
  virtual ~IParserStatementStrategy() {}
  virtual bool execute(ParserContext& ctx,
                       LexerLine* statement, Lexeme* lexeme) = 0;
};

#endif  // PARSER_STATEMENT_STRATEGY_H_INCLUDED
