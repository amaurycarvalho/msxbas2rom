#include "compiler_turbo_function_strategy.h"

#include "action_node.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int TurboCompilerFunctionStrategy::execute(CompilerContext* context,
                                           shared_ptr<ActionNode> action,
                                           int* result,
                                           unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 0) return Lexeme::subtype_unknown;

  (void)result;

  if (action->lexeme->value != "TURBO") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& optimizer = *context->codeOptimizer;

  // ld a, 5
  cpu.addLdA(5);
  // CALL USR2
  cpu.addCall(optimizer.getKernelCallAddr(def_usr2) + 1);

  return Lexeme::subtype_numeric;
}
