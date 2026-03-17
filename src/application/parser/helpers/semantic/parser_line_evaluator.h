#ifndef PARSER_LINE_EVALUATOR_H_INCLUDED
#define PARSER_LINE_EVALUATOR_H_INCLUDED

#include "assignment_evaluator.h"
#include "expression_evaluator.h"
#include "include_loader.h"
#include "parser_context.h"
#include "parser_statement_strategy_factory.h"

class LexerLineEvaluator;
class LexerLineContext;

class ParserLineEvaluator {
 public:
  ParserLineEvaluator(ParserContext& context,
                      ParserStatementStrategyFactory& strategyFactory,
                      ExpressionEvaluator& expressionEvaluator,
                      AssignmentEvaluator& assignmentEvaluator);
  ~ParserLineEvaluator();

  bool evaluateLine(LexerLineEvaluator* lexerLine);
  bool evaluatePhrase(LexerLineContext* phrase);
  bool evaluateStatement(LexerLineContext* statement);

 private:
  ParserContext& ctx;
  ParserStatementStrategyFactory& statementStrategyFactory;
  ExpressionEvaluator& exprEval;
  AssignmentEvaluator& assignEval;
  IncludeLoader includeLoader;
};

#endif  // PARSER_LINE_EVALUATOR_H_INCLUDED
