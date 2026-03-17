#ifndef COMPILER_CMD_DISSCR_HANDLER_H_INCLUDED
#define COMPILER_CMD_DISSCR_HANDLER_H_INCLUDED

#include "compiler_cmd_handler.h"

class CompilerCmdDisScrHandler : public ICompilerCmdHandler {
 public:
  bool execute(CompilerContext* context,
               shared_ptr<ActionNode> action) override;
};

#endif  // COMPILER_CMD_DISSCR_HANDLER_H_INCLUDED
