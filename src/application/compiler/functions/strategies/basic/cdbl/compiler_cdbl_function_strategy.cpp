#include "compiler_cdbl_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "lexeme.h"

int CdblCompilerFunctionStrategy::execute(CompilerContext* context,
                                          shared_ptr<ActionNode> action,
                                          int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "CDBL") return Lexeme::subtype_unknown;

  auto& expression = *context->expressionEvaluator;

  // cast
  expression.addCast(result[0], Lexeme::subtype_double_decimal);
  result[0] = Lexeme::subtype_double_decimal;

  return result[0];
}
