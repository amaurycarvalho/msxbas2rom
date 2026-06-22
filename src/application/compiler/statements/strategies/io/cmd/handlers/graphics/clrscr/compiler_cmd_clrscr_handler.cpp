#include "compiler_cmd_clrscr_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"

bool CompilerCmdClrScrHandler::execute(shared_ptr<CompilerContext> context,
                                       shared_ptr<ActionNode> action) {
  // call cmd_clrscr
  context->codeOptimizer->addKernelCall(DISP_cmd_clrscr);

  return context->compiled;
}
