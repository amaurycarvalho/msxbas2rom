#include "compiler_cmd_updfntclr_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"

bool CompilerCmdUpdFntClrHandler::execute(shared_ptr<CompilerContext> context,
                                          shared_ptr<ActionNode> action) {
  // call cmd_updfntclr
  context->codeOptimizer->addKernelCall(DISP_cmd_updfntclr);

  return context->compiled;
}
