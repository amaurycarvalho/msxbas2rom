#include "compiler_paint_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

void CompilerPaintStatementStrategy::cmd_paint(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<ActionNode> action, sub_action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;
  bool has_x_coord = false, has_y_coord = false, has_color = false,
       has_border = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];

      switch (i) {
        case 0: {
          if (action->actions.size() != 2) {
            context->syntaxError("Coordenates parameters error on PAINT");
            return;
          } else if (action->lexeme->value == "COORD") {
            sub_action = action->actions[0];

            result_subtype = expression.evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              expression.addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              cpu.addLdiiHL(0xFCB7);
              // push hl
              cpu.addPushHL();

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = expression.evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              expression.addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              cpu.addLdiiHL(0xFCB9);

              // push hl
              cpu.addPushHL();

              has_y_coord = true;
            }

          } else if (action->lexeme->value == "STEP") {
            sub_action = action->actions[0];

            result_subtype = expression.evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              expression.addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB7)  ;GRPACX
              cpu.addLdDEii(0xFCB7);
              // add hl, de
              cpu.addAddHLDE();
              // ld (0xFCB7), hl  ;GRPACX
              cpu.addLdiiHL(0xFCB7);
              // push hl
              cpu.addPushHL();

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = expression.evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              expression.addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB9)  ;GRPACY
              cpu.addLdDEii(0xFCB9);
              // add hl, de
              cpu.addAddHLDE();
              // ld (0xFCB9), hl  ;GRPACY
              cpu.addLdiiHL(0xFCB9);
              // push hl
              cpu.addPushHL();

              has_y_coord = true;
            }

          } else {
            context->syntaxError("Invalid coordenates on PAINT");
            return;
          }

        } break;

        case 1: {
          result_subtype = expression.evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld b, l       ; paint color
          cpu.addLdBL();

          has_color = true;

        } break;

        case 2: {
          if (has_color) {
            // push bc    ; save paint color
            cpu.addPushBC();
          }

          result_subtype = expression.evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          has_border = true;

        } break;

        default: {
          context->syntaxError("PAINT parameters not supported");
          return;
        }
      }
    }

    if (has_border) {
      if (has_color) {
        // pop bc               ; restore paint color
        cpu.addPopBC();
      } else {
        // ld a, (ATRBYT)
        cpu.addLdAii(def_ATRBYT);
        // ld b, a              ; paint color = default color
        cpu.addLdBA();
      }
      //   ld a, l                ; border color
      cpu.addLdAL();

    } else {
      if (has_color) {
        // ld a, l              ; border color = paint color
        cpu.addLdAL();
      } else {
        // ld a, (ATRBYT)       ; border color = default color
        cpu.addLdAii(def_ATRBYT);
        // ld b, a              ; paint color = default color
        cpu.addLdBA();
      }
    }

    if (has_y_coord) {
      // pop hl
      cpu.addPopHL();
    } else {
      // ld hl, (0xFCB9)  ;GRPACY
      cpu.addLdHLii(0xFCB9);
    }

    if (has_x_coord) {
      // pop de
      cpu.addPopDE();
    } else {
      // ld de, (0xFCB7)  ;GRPACX
      cpu.addLdDEii(0xFCB7);
    }

    // call 0x74B3   ; xbasic PAINT (in: hl=y, de=x, b=filling color, a=border
    // color)
    cpu.addCall(def_XBASIC_PAINT);

  } else {
    context->syntaxError("PAINT with empty parameters");
  }
}

bool CompilerPaintStatementStrategy::execute(CompilerContext* context) {
  cmd_paint(context);
  return context->compiled;
}
