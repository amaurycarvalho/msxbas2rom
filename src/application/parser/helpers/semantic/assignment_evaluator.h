#ifndef ASSIGNMENT_EVALUATOR_H_INCLUDED
#define ASSIGNMENT_EVALUATOR_H_INCLUDED

#include "expression_evaluator.h"
#include "parser_context.h"

using namespace std;

class AssignmentEvaluator {
 public:
  AssignmentEvaluator(ParserContext& context, ExpressionEvaluator& exprEval);

  /***
   * @brief Assignments syntatic analysis
   * @return True, if syntatic analysis success
   */
  bool evaluate(LexerLine* assignment);

 private:
  ParserContext& ctx;
  ExpressionEvaluator& exprEval;
};

#endif  // ASSIGNMENT_EVALUATOR_H_INCLUDED