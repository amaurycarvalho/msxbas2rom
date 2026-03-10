#include "compiler_cmd_clrscr_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"

bool CompilerCmdClrScrHandler::execute(CompilerContext* context,
                                       ActionNode* action) {
  auto& cpu = *context->cpu;

  // call cmd_clrscr
  cpu.addCall(def_cmd_clrscr);

  return context->compiled;
}
