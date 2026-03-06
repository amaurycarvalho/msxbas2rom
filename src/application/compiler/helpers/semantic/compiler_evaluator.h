/***
 * @file compiler_evaluator.h
 * @brief Compiler evaluator
 */

#ifndef COMPILER_EVALUATOR_H
#define COMPILER_EVALUATOR_H

#include "compiler_statement_strategy_factory.h"

class CompilerContext;
class TagNode;
class ActionNode;

using namespace std;

class CompilerEvaluator {
 public:
  CompilerContext* context;

  CompilerStatementStrategyFactory statementStrategyFactory;

  /***
   * @brief Perform a semanthic analysis on the specified tag node
   * @param tag TagNode object (action list)
   * @return True, if semanthic analysis success
   */
  bool evaluate(TagNode* tag);

  bool evalAction(ActionNode* action);
  bool evalActions(ActionNode* action);

  CompilerEvaluator(CompilerContext* context);
};

#endif  // COMPILER_EVALUATOR_H