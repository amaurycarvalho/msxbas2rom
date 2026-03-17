#include "compiler_cmd_mute_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"

bool CompilerCmdMuteHandler::execute(CompilerContext* context,
                                     shared_ptr<ActionNode> action) {
  auto& cpu = *context->cpu;

  // call cmd_mute
  cpu.addCall(def_cmd_mute);

  return context->compiled;
}
