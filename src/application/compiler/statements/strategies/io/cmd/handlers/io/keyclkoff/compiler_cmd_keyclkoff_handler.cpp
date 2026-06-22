#include "compiler_cmd_keyclkoff_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"

bool CompilerCmdKeyClkOffHandler::execute(shared_ptr<CompilerContext> context,
                                          shared_ptr<ActionNode> action) {
  auto& cpu = *context->cpu;

  // call cmd_keyclkoff
  context->codeOptimizer->addKernelDispatch(DISP_cmd_keyclkoff);

  return context->compiled;
}
