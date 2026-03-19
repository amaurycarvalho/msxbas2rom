#ifndef COMPILER_CMD_MTF_HANDLER_H_INCLUDED
#define COMPILER_CMD_MTF_HANDLER_H_INCLUDED

#include "compiler_cmd_handler.h"

class CompilerCmdMtfHandler : public ICompilerCmdHandler {
 public:
  /***
   * @brief CMD MTF: MSX Tile Forge support
   * @param context Compiler context
   * @param action Current action node
   */
  bool execute(shared_ptr<CompilerContext> context,
               shared_ptr<ActionNode> action) override;
};

#endif  // COMPILER_CMD_MTF_HANDLER_H_INCLUDED
