#ifndef COMPILER_CMD_WRTCHR_HANDLER_H_INCLUDED
#define COMPILER_CMD_WRTCHR_HANDLER_H_INCLUDED

#include "compiler_cmd_handler.h"

class CompilerCmdWrtChrHandler : public ICompilerCmdHandler {
 public:
  bool execute(CompilerContext* context, ActionNode* action) override;
};

#endif  // COMPILER_CMD_WRTCHR_HANDLER_H_INCLUDED
