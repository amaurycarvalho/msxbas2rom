/***
 * @file compiler_variable_emitter.h
 * @brief Compiler variable emitter
 */

#ifndef COMPILER_VARIABLE_EMITTER_H
#define COMPILER_VARIABLE_EMITTER_H

#include <memory>

class CompilerContext;
class ActionNode;

using namespace std;

class CompilerVariableEmitter {
 private:
  shared_ptr<CompilerContext> context;

 public:
  bool addVarAddress(shared_ptr<ActionNode> action);
  void addTempStr(bool atHL);
  bool addAssignment(shared_ptr<ActionNode> action);

  CompilerVariableEmitter(shared_ptr<CompilerContext> context);
  ~CompilerVariableEmitter();
};

#endif  // COMPILER_VARIABLE_EMITTER_H