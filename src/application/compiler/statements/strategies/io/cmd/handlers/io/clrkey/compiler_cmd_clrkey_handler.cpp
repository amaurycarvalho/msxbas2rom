#include "compiler_cmd_clrkey_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"

bool CompilerCmdClrKeyHandler::execute(shared_ptr<CompilerContext> context,
                                       shared_ptr<ActionNode> action) {
  // call cmd_clrkey
  context->codeOptimizer->addKernelCall(DISP_cmd_clrkey);

  return context->compiled;
}
