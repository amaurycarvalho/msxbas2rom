#include "compiler_cmd_clrkey_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"

bool CompilerCmdClrKeyHandler::execute(shared_ptr<CompilerContext> context,
                                       shared_ptr<ActionNode> action) {
  auto& cpu = *context->cpu;

  // call cmd_clrkey
  cpu.addCall(def_cmd_clrkey);

  return context->compiled;
}
