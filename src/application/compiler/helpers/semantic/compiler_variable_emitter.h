/***
 * @file compiler_variable_emitter.h
 * @brief Compiler variable emitter
 */

#ifndef COMPILER_VARIABLE_EMITTER_H
#define COMPILER_VARIABLE_EMITTER_H

class CompilerContext;
class ActionNode;

class CompilerVariableEmitter {
 private:
  CompilerContext* context;

 public:
  bool addVarAddress(ActionNode* action);
  void addTempStr(bool atHL);
  bool addAssignment(ActionNode* action);

  CompilerVariableEmitter(CompilerContext* context);
  ~CompilerVariableEmitter();
};

#endif  // COMPILER_VARIABLE_EMITTER_H