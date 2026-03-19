#include "compiler_resourcesize_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int ResourcesizeCompilerFunctionStrategy::execute(
    shared_ptr<CompilerContext> context, shared_ptr<ActionNode> action,
    int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "RESOURCESIZE") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;

  // cast
  expression.addCast(result[0], Lexeme::subtype_numeric);
  result[0] = Lexeme::subtype_numeric;

  // call usr1
  cpu.addCall(def_usr1);

  return result[0];
}
