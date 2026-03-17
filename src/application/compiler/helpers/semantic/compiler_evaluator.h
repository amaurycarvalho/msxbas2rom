/***
 * @file compiler_evaluator.h
 * @brief Compiler evaluator
 */

#ifndef COMPILER_EVALUATOR_H
#define COMPILER_EVALUATOR_H

#include <functional>
#include <memory>

class CompilerContext;
class TagNode;
class ActionNode;
class CompilerStatementStrategyFactory;

using namespace std;

class CompilerEvaluator {
 private:
  CompilerContext* context;
  unique_ptr<CompilerStatementStrategyFactory> statementFactory;

 public:
  /***
   * @brief Perform a semanthic analysis on the specified tag node
   * @param tag TagNode object (action list)
   * @return True, if semanthic analysis success
   */
  bool evaluate(shared_ptr<TagNode> tag);

  bool evalAction(shared_ptr<ActionNode> action);
  bool evalActions(shared_ptr<ActionNode> action);

  CompilerEvaluator(CompilerContext* context);
  ~CompilerEvaluator();
};

#endif  // COMPILER_EVALUATOR_H