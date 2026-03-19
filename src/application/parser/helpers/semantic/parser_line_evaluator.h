#ifndef PARSER_LINE_EVALUATOR_H_INCLUDED
#define PARSER_LINE_EVALUATOR_H_INCLUDED

#include <memory>

class ParserContext;
class LexerLineEvaluator;
class LexerLineContext;
class ParserStatementStrategyFactory;
class ExpressionEvaluator;
class AssignmentEvaluator;
class IncludeLoader;

using namespace std;

class ParserLineEvaluator {
 public:
  ParserLineEvaluator(shared_ptr<ParserContext> context,
                      shared_ptr<ExpressionEvaluator> expressionEvaluator,
                      shared_ptr<AssignmentEvaluator> assignmentEvaluator,
                      ParserStatementStrategyFactory* strategyFactory);
  ~ParserLineEvaluator();

  bool evaluateLine(shared_ptr<LexerLineEvaluator> lexerLine);
  bool evaluatePhrase(shared_ptr<LexerLineContext> phrase);
  bool evaluateStatement(shared_ptr<LexerLineContext> statement);

  shared_ptr<ParserContext> getContext();
  void setContext(shared_ptr<ParserContext> context);

 private:
  shared_ptr<ParserContext> ctx;
  shared_ptr<ExpressionEvaluator> exprEval;
  shared_ptr<AssignmentEvaluator> assignEval;
  ParserStatementStrategyFactory* statementStrategyFactory;
  unique_ptr<IncludeLoader> includeLoader;
};

#endif  // PARSER_LINE_EVALUATOR_H_INCLUDED
