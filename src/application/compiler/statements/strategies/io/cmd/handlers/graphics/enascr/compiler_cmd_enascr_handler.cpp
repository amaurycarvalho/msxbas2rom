#include "compiler_cmd_enascr_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"

bool CompilerCmdEnaScrHandler::execute(shared_ptr<CompilerContext> context,
                                       shared_ptr<ActionNode> action) {
  // call cmd_enascr
  context->codeOptimizer->addKernelCall(DISP_cmd_enascr);

  return context->compiled;
}
