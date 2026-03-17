#include "compiler_inp_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "lexeme.h"

int InpCompilerFunctionStrategy::execute(CompilerContext* context,
                                         shared_ptr<ActionNode> action,
                                         int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;

  if (action->lexeme->value != "INP") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;

  if (result[0] == Lexeme::subtype_single_decimal ||
      result[0] == Lexeme::subtype_double_decimal) {
    // cast
    expression.addCast(result[0], Lexeme::subtype_numeric);
    result[0] = Lexeme::subtype_numeric;
  }

  if (result[0] == Lexeme::subtype_numeric) {
    // ld c, l
    cpu.addLdCL();
    // in a, (c)
    cpu.addWord(0xED, 0x78);
    // ld l, a
    cpu.addLdLA();
    // ld h, 0
    cpu.addLdH(0x00);

  } else
    result[0] = Lexeme::subtype_unknown;

  return result[0];
}
