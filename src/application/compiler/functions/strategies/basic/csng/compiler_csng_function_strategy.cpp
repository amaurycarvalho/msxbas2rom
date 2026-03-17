#include "compiler_csng_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "lexeme.h"

int CsngCompilerFunctionStrategy::execute(CompilerContext* context,
                                          shared_ptr<ActionNode> action,
                                          int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "CSNG") return Lexeme::subtype_unknown;

  auto& expression = *context->expressionEvaluator;

  // cast
  expression.addCast(result[0], Lexeme::subtype_single_decimal);
  result[0] = Lexeme::subtype_single_decimal;

  return result[0];
}
