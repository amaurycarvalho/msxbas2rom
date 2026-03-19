#include "compiler_cmd_enascr_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"

bool CompilerCmdEnaScrHandler::execute(shared_ptr<CompilerContext> context,
                                       shared_ptr<ActionNode> action) {
  auto& cpu = *context->cpu;

  // call cmd_enascr
  cpu.addCall(def_cmd_enascr);

  return context->compiled;
}
