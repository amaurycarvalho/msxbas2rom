#include "compiler_plystatus_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int PlystatusCompilerFunctionStrategy::execute(CompilerContext* context,
                                               ActionNode* action,
                                               int* result,
                                               unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 0) return Lexeme::subtype_unknown;

  (void)result;

  if (action->lexeme->value != "PLYSTATUS") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;

  // CALL usr2_player_status
  cpu.addCall(def_usr2_player_status);

  return Lexeme::subtype_numeric;
}
