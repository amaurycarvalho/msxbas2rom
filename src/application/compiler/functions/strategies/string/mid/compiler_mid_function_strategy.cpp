#include "compiler_mid_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "compiler_variable_emitter.h"
#include "lexeme.h"

int MidCompilerFunctionStrategy::execute(CompilerContext* context,
                                         shared_ptr<ActionNode> action,
                                         int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "MID$") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& variable = *context->variableEmitter;
  auto& expression = *context->expressionEvaluator;

  switch (parmCount) {
    case 2: {
      if (result[0] == Lexeme::subtype_single_decimal ||
          result[0] == Lexeme::subtype_double_decimal) {
        // cast
        expression.addCast(result[0], Lexeme::subtype_numeric);
        result[0] = Lexeme::subtype_numeric;
      }

      if (result[1] == Lexeme::subtype_string &&
          result[0] == Lexeme::subtype_numeric) {
        // ld b, l         ; start char
        cpu.addLdBL();
        // pop hl          ; source string
        cpu.addPopHL();

        // ld a, 0xff      ; number of chars (all left on source string)
        cpu.addLdA(0xff);

        // call 0x7db1     ; xbasic mid string (in: b=start, a=size,
        // hl=source; out: hl=BUF)
        cpu.addCall(def_XBASIC_MID);

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

    case 3: {
      if (result[0] == Lexeme::subtype_single_decimal ||
          result[0] == Lexeme::subtype_double_decimal) {
        // cast
        expression.addCast(result[0], Lexeme::subtype_numeric);
        result[0] = Lexeme::subtype_numeric;
      }

      // ld a, l             ; number of chars
      cpu.addLdAL();

      if (result[1] == Lexeme::subtype_single_decimal ||
          result[1] == Lexeme::subtype_double_decimal) {
        // pop bc
        cpu.addPopBC();
        // pop hl
        cpu.addPopHL();
        // push af
        cpu.addPushAF();
        // cast
        expression.addCast(result[1], Lexeme::subtype_numeric);
        // pop af
        cpu.addPopAF();
        result[1] = Lexeme::subtype_numeric;
      } else {
        // pop hl
        cpu.addPopHL();
      }

      if (result[2] == Lexeme::subtype_string &&
          result[1] == Lexeme::subtype_numeric &&
          result[0] == Lexeme::subtype_numeric) {
        // ld b, l         ; start char
        cpu.addLdBL();
        // pop hl          ; source string
        cpu.addPopHL();

        // call 0x7db1     ; xbasic mid string (in: b=start, a=size,
        // hl=source; out: hl=BUF)
        cpu.addCall(def_XBASIC_MID);

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

    default:
      return Lexeme::subtype_unknown;
  }
}
