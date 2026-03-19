#ifndef ASSIGNMENT_EVALUATOR_H_INCLUDED
#define ASSIGNMENT_EVALUATOR_H_INCLUDED

#include <memory>

class ParserContext;
class ExpressionEvaluator;
class LexerLineContext;

using namespace std;

class AssignmentEvaluator {
 public:
  AssignmentEvaluator(shared_ptr<ParserContext> context,
                      shared_ptr<ExpressionEvaluator> exprEval);
  ~AssignmentEvaluator();

  /***
   * @brief Assignments syntatic analysis
   * @return True, if syntatic analysis success
   */
  bool evaluate(shared_ptr<LexerLineContext> assignment);

  shared_ptr<ParserContext> getContext();
  void setContext(shared_ptr<ParserContext> context);

 private:
  shared_ptr<ParserContext> ctx;
  shared_ptr<ExpressionEvaluator> exprEval;
};

#endif  // ASSIGNMENT_EVALUATOR_H_INCLUDED