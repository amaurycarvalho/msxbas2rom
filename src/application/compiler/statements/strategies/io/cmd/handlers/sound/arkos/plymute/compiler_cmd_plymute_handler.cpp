/***
 * @file compiler_cmd_plymute_handler.cpp
 * @brief Arkos Tracker handler
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_cmd_plymute_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"

bool CompilerCmdPlyMuteHandler::execute(shared_ptr<CompilerContext> context,
                                        shared_ptr<ActionNode> action) {
  auto& cpu = *context->cpu;

  context->akm = true;

  // call cmd_akmmute
  cpu.addCall(def_cmd_plymute);

  return context->compiled;
}
