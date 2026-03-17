#include "compiler_vdp_function_strategy.h"

#include "action_node.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

int VdpCompilerFunctionStrategy::execute(CompilerContext* context,
                                         shared_ptr<ActionNode> action,
                                         int* result, unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "VDP") return Lexeme::subtype_unknown;

  auto& cpu = *context->cpu;
  auto& optimizer = *context->codeOptimizer;

  switch (parmCount) {
    case 0: {
      // VDP() without parameters returns VDP version
      // ld a, 4
      cpu.addLdA(4);
      // CALL USR2
      cpu.addCall(optimizer.getKernelCallAddr(def_usr2) + 1);
      return Lexeme::subtype_numeric;
    }

    case 1: {
      auto& expression = *context->expressionEvaluator;

      if (result[0] == Lexeme::subtype_single_decimal ||
          result[0] == Lexeme::subtype_double_decimal) {
        // cast
        expression.addCast(result[0], Lexeme::subtype_numeric);
        result[0] = Lexeme::subtype_numeric;
      }

      if (result[0] == Lexeme::subtype_numeric) {
        // call 0x7337         ; xbasic VDP (in: hl, out: hl)
        cpu.addCall(def_XBASIC_VDP);
      } else
        result[0] = Lexeme::subtype_unknown;

      return result[0];
    }

    default:
      return Lexeme::subtype_unknown;
  }
}
