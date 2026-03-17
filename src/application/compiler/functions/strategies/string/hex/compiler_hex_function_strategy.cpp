#include "compiler_hex_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "compiler_variable_emitter.h"
#include "lexeme.h"

int HexCompilerFunctionStrategy::execute(CompilerContext* context,
                                         shared_ptr<ActionNode> action,
                                         int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount != 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "HEX$") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& variable = *context->variableEmitter;
  auto& expression = *context->expressionEvaluator;

  if (result[0] == Lexeme::subtype_single_decimal ||
      result[0] == Lexeme::subtype_double_decimal) {
    // cast
    expression.addCast(result[0], Lexeme::subtype_numeric);
    result[0] = Lexeme::subtype_numeric;
  }

  if (result[0] == Lexeme::subtype_numeric) {
    // ld c, 4
    cpu.addLdC(0x04);
    // call 0x7e22    ; xbasic OCT$/HEX$/BIN$ (in: hl=integer, de=BUF,
    // c=mode [1=bin, 3=oct, 4=hex]; out: hl destination corrected)
    cpu.addCall(def_XBASIC_OCT_HEX_BIN);
    // ld de, temporary string
    variable.addTempStr(false);
    // push de
    cpu.addPushDE();
    //   call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest;
    //   out: hl end of string)
    cpu.addCall(def_XBASIC_COPY_STRING);
    // pop hl
    cpu.addPopHL();

    return Lexeme::subtype_string;
  }

  return Lexeme::subtype_unknown;
}
