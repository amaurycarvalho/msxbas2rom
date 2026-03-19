#ifndef COMPILER_CMD_MUTE_HANDLER_H_INCLUDED
#define COMPILER_CMD_MUTE_HANDLER_H_INCLUDED

#include "compiler_cmd_handler.h"

class CompilerCmdMuteHandler : public ICompilerCmdHandler {
 public:
  bool execute(shared_ptr<CompilerContext> context,
               shared_ptr<ActionNode> action) override;
};

#endif  // COMPILER_CMD_MUTE_HANDLER_H_INCLUDED
