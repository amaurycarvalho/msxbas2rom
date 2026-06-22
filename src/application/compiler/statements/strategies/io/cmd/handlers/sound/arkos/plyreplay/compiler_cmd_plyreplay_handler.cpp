/***
 * @file compiler_cmd_plyreplay_handler.cpp
 * @brief Arkos Tracker handler
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_cmd_plyreplay_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"

bool CompilerCmdPlyReplayHandler::execute(shared_ptr<CompilerContext> context,
                                          shared_ptr<ActionNode> action) {
  context->akm = true;

  // call cmd_plyreplay
  context->codeOptimizer->addKernelCall(DISP_cmd_plyreplay);

  return context->compiled;
}
