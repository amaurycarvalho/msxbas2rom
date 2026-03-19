#ifndef COMPILER_CMD_RAMTORAM_HANDLER_H_INCLUDED
#define COMPILER_CMD_RAMTORAM_HANDLER_H_INCLUDED

#include "compiler_cmd_handler.h"

class CompilerCmdRamToRamHandler : public ICompilerCmdHandler {
 public:
  bool execute(shared_ptr<CompilerContext> context,
               shared_ptr<ActionNode> action) override;
};

#endif  // COMPILER_CMD_RAMTORAM_HANDLER_H_INCLUDED
