/***
 * @file compiler_expression_evaluator.h
 * @brief Compiler expression evaluator
 */

#ifndef COMPILER_EXPRESSION_EVALUATOR_H
#define COMPILER_EXPRESSION_EVALUATOR_H

class CompilerContext;
class ActionNode;

class CompilerExpressionEvaluator {
 private:
  CompilerContext* context;

 public:
  int evalExpression(ActionNode* action);
  int evalOperator(ActionNode* action);
  int evalFunction(ActionNode* action);
  bool evalOperatorParms(ActionNode* action, int parmCount);
  int evalOperatorCast(ActionNode* action);
  void addCast(int from, int to);

  CompilerExpressionEvaluator(CompilerContext* context) : context(context) {};
};

#endif  // COMPILER_EXPRESSION_EVALUATOR_H