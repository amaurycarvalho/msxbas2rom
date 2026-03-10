#ifndef COMPILER_CMD_PLYLOOP_HANDLER_H_INCLUDED
#define COMPILER_CMD_PLYLOOP_HANDLER_H_INCLUDED

#include "compiler_cmd_handler.h"

class CompilerCmdPlyLoopHandler : public ICompilerCmdHandler {
 public:
  /***
   * @brief CMD PLYLOOP: Arkos Tracker support
   * @param context Compiler context
   * @param action Current action node
   */
  bool execute(CompilerContext* context, ActionNode* action) override;
};

#endif  // COMPILER_CMD_PLYLOOP_HANDLER_H_INCLUDED
