#ifndef COMPILER_CMD_PLYPLAY_HANDLER_H_INCLUDED
#define COMPILER_CMD_PLYPLAY_HANDLER_H_INCLUDED

#include "compiler_cmd_handler.h"

class CompilerCmdPlyPlayHandler : public ICompilerCmdHandler {
 public:
  /***
   * @brief CMD PLYPLAY: Arkos Tracker support
   * @param context Compiler context
   * @param action Current action node
   */
  bool execute(CompilerContext* context, ActionNode* action) override;
};

#endif  // COMPILER_CMD_PLYPLAY_HANDLER_H_INCLUDED
