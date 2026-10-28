#include "compiler_space_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_code_optimizer.h"
#include "compiler_hooks.h"
#include "compiler_variable_emitter.h"
#include "lexeme.h"

int SpaceCompilerFunctionStrategy::execute(shared_ptr<CompilerContext> context,
                                           shared_ptr<ActionNode> action,
                                           int* result,
                                            unsigned int parmCount) {
  auto& optimizer = *context->getCodeOptimizer();
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "SPACE$" && action->lexeme->value != "SPC")
    return Lexeme::subtype_unknown;

  auto& cpu = *context->getCpu();
  auto& variable = *context->getVariableEmitter();
  auto& expression = *context->getExpressionEvaluator();

  if (result[0] == Lexeme::subtype_single_decimal ||
      result[0] == Lexeme::subtype_double_decimal) {
    // cast
    expression.addCast(result[0], Lexeme::subtype_numeric);
    result[0] = Lexeme::subtype_numeric;
  }

  if (result[0] == Lexeme::subtype_numeric) {
    // ld a, 0x20      ; space
    cpu.addLdA(0x20);
    // ld b, l
    cpu.addLdBL();
    // call 0x7e4c    ; STRING$ (b=number of chars, a=char; out: hl=BUF)
    optimizer.addKernelCall(DISP_XBASIC_STRING);
    // ld de, temporary string
    variable.addTempStr(false);
    // push de
    cpu.addPushDE();
    //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
    //   out: hl end of string)
    optimizer.addKernelCall(DISP_XBASIC_COPY_STRING);
    // pop hl
    cpu.addPopHL();

    return Lexeme::subtype_string;
  }

  return Lexeme::subtype_unknown;
}
