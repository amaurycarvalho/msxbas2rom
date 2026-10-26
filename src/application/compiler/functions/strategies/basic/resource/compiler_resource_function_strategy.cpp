#include "compiler_resource_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int ResourceCompilerFunctionStrategy::execute(
    shared_ptr<CompilerContext> context, shared_ptr<ActionNode> action,
    int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "RESOURCE") return Lexeme::subtype_unknown;

  auto& cpu = *context->getCpu();
  auto& expression = *context->getExpressionEvaluator();

  expression.addCast(result[0], Lexeme::subtype_numeric);
  result[0] = Lexeme::subtype_numeric;

  cpu.addLdiiHL(def_RSC_RESOURCE);

  cpu.addLdHLii(def_HEAPSTR);
  cpu.addLdiiHL(def_RSC_DEST);

  cpu.addLdHL(0);
  cpu.addLdiiHL(def_RSC_PLETTER);

  cpu.addLdHL(0);
  cpu.addLdiiHL(def_RSC_OFFSET);

  cpu.addLdHLii(def_HEAPSIZ);
  cpu.addLdiiHL(def_RSC_SIZE);

  context->getCodeOptimizer()->addKernelCall(DISP_cmd_rsctoram);

  return result[0];
}
