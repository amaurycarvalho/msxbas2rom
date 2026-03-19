#ifndef COMPILER_CMD_HANDLER_H_INCLUDED
#define COMPILER_CMD_HANDLER_H_INCLUDED

#include <memory>

class CompilerContext;
class ActionNode;

using namespace std;

class ICompilerCmdHandler {
 public:
  virtual ~ICompilerCmdHandler() {}
  virtual bool execute(shared_ptr<CompilerContext> context,
                       shared_ptr<ActionNode> action) = 0;
};

#endif  // COMPILER_CMD_HANDLER_H_INCLUDED
