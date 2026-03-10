#ifndef COMPILER_CMD_PLYMUTE_HANDLER_H_INCLUDED
#define COMPILER_CMD_PLYMUTE_HANDLER_H_INCLUDED

#include "compiler_cmd_handler.h"

class CompilerCmdPlyMuteHandler : public ICompilerCmdHandler {
 public:
  /***
   * @brief CMD PLYMUTE: Arkos Tracker support
   * @param context Compiler context
   * @param action Current action node
   */
  bool execute(CompilerContext* context, ActionNode* action) override;
};

#endif  // COMPILER_CMD_PLYMUTE_HANDLER_H_INCLUDED
