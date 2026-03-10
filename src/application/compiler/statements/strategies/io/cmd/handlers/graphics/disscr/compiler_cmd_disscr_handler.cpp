#include "compiler_cmd_disscr_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"

bool CompilerCmdDisScrHandler::execute(CompilerContext* context,
                                       ActionNode* action) {
  auto& cpu = *context->cpu;

  // call cmd_disscr
  cpu.addCall(def_cmd_disscr);

  return context->compiled;
}
