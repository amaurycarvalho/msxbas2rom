#include "compiler_str_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "lexeme.h"

int StrCompilerFunctionStrategy::execute(CompilerContext* context,
                                         ActionNode* action,
                                         int* result,
                                         unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "STR$") return Lexeme::subtype_unknown;

  auto& expression = *context->expressionEvaluator;

  // cast
  expression.addCast(result[0], Lexeme::subtype_string);
  result[0] = Lexeme::subtype_string;

  return result[0];
}
