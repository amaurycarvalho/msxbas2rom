#ifndef COMPILER_CMD_HANDLER_H_INCLUDED
#define COMPILER_CMD_HANDLER_H_INCLUDED

class CompilerContext;
class ActionNode;

class ICompilerCmdHandler {
 public:
  virtual ~ICompilerCmdHandler() {}
  virtual bool execute(CompilerContext* context, ActionNode* action) = 0;
};

#endif  // COMPILER_CMD_HANDLER_H_INCLUDED
