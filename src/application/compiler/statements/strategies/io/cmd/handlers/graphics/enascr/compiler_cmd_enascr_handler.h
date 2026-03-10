#ifndef COMPILER_CMD_ENASCR_HANDLER_H_INCLUDED
#define COMPILER_CMD_ENASCR_HANDLER_H_INCLUDED

#include "compiler_cmd_handler.h"

class CompilerCmdEnaScrHandler : public ICompilerCmdHandler {
 public:
  bool execute(CompilerContext* context, ActionNode* action) override;
};

#endif  // COMPILER_CMD_ENASCR_HANDLER_H_INCLUDED
