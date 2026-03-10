#include "compiler_wait_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_expression_evaluator.h"

void CompilerWaitStatementStrategy::cmd_wait(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t == 2 || t == 3) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme) {
        result_subtype = expression.evalExpression(action);

        // cast
        expression.addCast(result_subtype, Lexeme::subtype_numeric);

        switch (i) {
          case 0: {
            // push hl
            cpu.addPushHL();
          } break;

          case 1: {
            if (t == 3) {
              // ld a, l
              cpu.addLdAL();
              // push af
              cpu.addPushAF();
            } else {
              // ld d, l
              cpu.addLdDL();
              // pop bc
              cpu.addPopBC();
              //   in a,(c)
              cpu.addWord(0xED, 0x78);
              //   and d
              cpu.addAndD();
              // jr z, $-4
              cpu.addJrZ(0xFB);
            }
          } break;

          case 2: {
            // ld h, l
            cpu.addLdHL();
            // pop de
            cpu.addPopDE();
            // pop bc
            cpu.addPopBC();
            //   in a,(c)
            cpu.addWord(0xED, 0x78);
            //   xor h
            cpu.addXorH();
            //   and d
            cpu.addAndD();
            // jr z, $-5
            cpu.addJrZ(0xFA);

          } break;
        }
      }
    }

  } else {
    context->syntaxError("Invalid WAIT parameters");
  }
}

bool CompilerWaitStatementStrategy::execute(CompilerContext* context) {
  cmd_wait(context);
  return context->compiled;
}
