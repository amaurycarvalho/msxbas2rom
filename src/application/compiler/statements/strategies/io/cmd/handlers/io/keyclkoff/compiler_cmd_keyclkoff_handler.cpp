#include "compiler_cmd_keyclkoff_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"

bool CompilerCmdKeyClkOffHandler::execute(CompilerContext* context,
                                          shared_ptr<ActionNode> action) {
  auto& cpu = *context->cpu;

  // call cmd_keyclkoff
  cpu.addCall(def_cmd_keyclkoff);

  return context->compiled;
}
