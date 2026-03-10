/***
 * @file compiler_expression_evaluator.h
 * @brief Compiler expression evaluator
 */

#ifndef COMPILER_EXPRESSION_EVALUATOR_H
#define COMPILER_EXPRESSION_EVALUATOR_H

#include <functional>
#include <memory>

class CompilerContext;
class ActionNode;
class CompilerFunctionStrategyFactory;

using namespace std;

class CompilerExpressionEvaluator {
 private:
  CompilerContext* context;
  unique_ptr<CompilerFunctionStrategyFactory> functionFactory;

 public:
  int evalExpression(ActionNode* action);
  int evalOperator(ActionNode* action);
  int evalFunction(ActionNode* action);
  bool evalOperatorParms(ActionNode* action, int parmCount);
  int evalOperatorCast(ActionNode* action);
  void addCast(int from, int to);

  CompilerExpressionEvaluator(CompilerContext* context);
  ~CompilerExpressionEvaluator();
};

#endif  // COMPILER_EXPRESSION_EVALUATOR_H