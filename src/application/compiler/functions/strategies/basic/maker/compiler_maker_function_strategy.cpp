#include "compiler_maker_function_strategy.h"

#include "action_node.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int MakerCompilerFunctionStrategy::execute(shared_ptr<CompilerContext> context,
                                           shared_ptr<ActionNode> action,
                                           int* result,
                                           unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 0) return Lexeme::subtype_unknown;

  (void)result;

  if (action->lexeme->value != "MAKER") return Lexeme::subtype_unknown;

  auto& cpu = *context->getCpu();
  // auto& optimizer = *context->getCodeOptimizer();

  // ld hl, 6 ; selector
  cpu.addLdHL(6);
  // CALL USR2 through kernel dispatch
  context->getCodeOptimizer()->addKernelCall(DISP_usr2);

  return Lexeme::subtype_numeric;
}
