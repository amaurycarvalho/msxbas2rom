#include "compiler_cmd_disscr_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"

bool CompilerCmdDisScrHandler::execute(shared_ptr<CompilerContext> context,
                                       shared_ptr<ActionNode> action) {
  // call cmd_disscr
  context->codeOptimizer->addKernelCall(DISP_cmd_disscr);

  return context->compiled;
}
