#ifndef COMPILER_CMD_WRTSPRCLR_HANDLER_H_INCLUDED
#define COMPILER_CMD_WRTSPRCLR_HANDLER_H_INCLUDED

#include "compiler_cmd_handler.h"

class CompilerCmdWrtSprClrHandler : public ICompilerCmdHandler {
 public:
  bool execute(CompilerContext* context,
               shared_ptr<ActionNode> action) override;
};

#endif  // COMPILER_CMD_WRTSPRCLR_HANDLER_H_INCLUDED
