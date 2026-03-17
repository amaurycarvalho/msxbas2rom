#include "compiler_instr_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int InstrCompilerFunctionStrategy::execute(CompilerContext* context,
                                           shared_ptr<ActionNode> action,
                                           int* result,
                                           unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "INSTR") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;

  switch (parmCount) {
    case 2: {
      if (result[0] == Lexeme::subtype_string &&
          result[1] == Lexeme::subtype_string) {
        // ex de,hl        ; search string
        cpu.addExDEHL();
        // pop hl          ; source string
        cpu.addPopHL();

        // ld a, 0x01      ; search start
        cpu.addLdA(0x01);

        // call 0x7e6c     ; xbasic INSTR (in: a=start, hl=source,
        // de=search; out: hl=position)
        cpu.addCall(def_XBASIC_INSTR);

        return Lexeme::subtype_numeric;
      }

      return Lexeme::subtype_unknown;
    }

    case 3: {
      // ex de,hl        ; search string
      cpu.addExDEHL();
      // pop hl          ; source string
      cpu.addPopHL();
      // pop bc          ; search start
      cpu.addPopBC();

      if (result[2] == Lexeme::subtype_single_decimal ||
          result[2] == Lexeme::subtype_double_decimal) {
        // ex (sp),hl
        cpu.addExiSPHL();
        // push de
        cpu.addPushDE();
        //   cast
        expression.addCast(result[2], Lexeme::subtype_numeric);
        //   ld c, l
        cpu.addLdCL();
        // pop de
        cpu.addPopDE();
        // pop hl
        cpu.addPopHL();
        result[2] = Lexeme::subtype_numeric;
      }

      // ld a, c             ; search start
      cpu.addLdAC();

      if (result[0] == Lexeme::subtype_string &&
          result[1] == Lexeme::subtype_string &&
          result[2] == Lexeme::subtype_numeric) {
        // call 0x7e6c     ; xbasic INSTR (in: a=start, hl=source,
        // de=search; out: hl=position)
        cpu.addCall(def_XBASIC_INSTR);

        return Lexeme::subtype_numeric;
      }

      return Lexeme::subtype_unknown;
    }

    default:
      return Lexeme::subtype_unknown;
  }
}
