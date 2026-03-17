#ifndef COMPILER_CMD_PLYLOAD_HANDLER_H_INCLUDED
#define COMPILER_CMD_PLYLOAD_HANDLER_H_INCLUDED

#include "compiler_cmd_handler.h"

class CompilerCmdPlyLoadHandler : public ICompilerCmdHandler {
 public:
  /***
   * @brief CMD PLYLOAD: Arkos Tracker support
   * @param context Compiler context
   * @param action Current action node
   */
  bool execute(CompilerContext* context,
               shared_ptr<ActionNode> action) override;
};

#endif  // COMPILER_CMD_PLYLOAD_HANDLER_H_INCLUDED
