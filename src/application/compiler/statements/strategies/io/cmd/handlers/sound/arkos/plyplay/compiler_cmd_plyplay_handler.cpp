/***
 * @file compiler_cmd_plyplay_handler.cpp
 * @brief Arkos Tracker handler
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_cmd_plyplay_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"

bool CompilerCmdPlyPlayHandler::execute(CompilerContext* context,
                                        ActionNode* action) {
  auto& cpu = *context->cpu;

  context->akm = true;

  // call cmd_plyplay
  cpu.addCall(def_cmd_plyplay);

  return context->compiled;
}
