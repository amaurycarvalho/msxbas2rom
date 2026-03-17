#include "compiler_cmd_updfntclr_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"

bool CompilerCmdUpdFntClrHandler::execute(CompilerContext* context,
                                          shared_ptr<ActionNode> action) {
  auto& cpu = *context->cpu;

  // call cmd_updfntclr
  cpu.addCall(def_cmd_updfntclr);

  return context->compiled;
}
