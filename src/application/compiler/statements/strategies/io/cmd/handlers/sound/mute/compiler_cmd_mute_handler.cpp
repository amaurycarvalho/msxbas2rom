#include "compiler_cmd_mute_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"

bool CompilerCmdMuteHandler::execute(shared_ptr<CompilerContext> context,
                                     shared_ptr<ActionNode> action) {
  // call cmd_mute
  context->codeOptimizer->addKernelCall(DISP_cmd_mute);

  return context->compiled;
}
