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
  shared_ptr<CompilerContext> context;
  shared_ptr<CompilerFunctionStrategyFactory> functionFactory;

 public:
  int evalExpression(shared_ptr<ActionNode> action);
  int evalOperator(shared_ptr<ActionNode> action);
  int evalFunction(shared_ptr<ActionNode> action);
  bool evalOperatorParms(shared_ptr<ActionNode> action, int parmCount);
  int evalOperatorCast(shared_ptr<ActionNode> action);
  void addCast(int from, int to);

  CompilerExpressionEvaluator(shared_ptr<CompilerContext> context);
  ~CompilerExpressionEvaluator();
};

#endif  // COMPILER_EXPRESSION_EVALUATOR_H