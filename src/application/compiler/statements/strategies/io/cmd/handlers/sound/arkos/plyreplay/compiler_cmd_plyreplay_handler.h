#ifndef COMPILER_CMD_PLYREPLAY_HANDLER_H_INCLUDED
#define COMPILER_CMD_PLYREPLAY_HANDLER_H_INCLUDED

#include "compiler_cmd_handler.h"

class CompilerCmdPlyReplayHandler : public ICompilerCmdHandler {
 public:
  /***
   * @brief CMD PLYREPLAY: Arkos Tracker support
   * @param context Compiler context
   * @param action Current action node
   */
  bool execute(CompilerContext* context, ActionNode* action) override;
};

#endif  // COMPILER_CMD_PLYREPLAY_HANDLER_H_INCLUDED
