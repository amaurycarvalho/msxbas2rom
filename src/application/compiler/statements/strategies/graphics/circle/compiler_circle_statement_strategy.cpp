#include "compiler_circle_statement_strategy.h"

#include "action_node.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_evaluator.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

void CompilerCircleStatementStrategy::cmd_circle(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& optimizer = *context->codeOptimizer;
  shared_ptr<ActionNode> action, sub_action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;
  bool has_x_coord = false, has_y_coord = false, has_radius = false,
       has_color = false;
  bool has_trace1 = false, has_trace2 = false, has_aspect = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];

      switch (i) {
        // coord
        case 0: {
          if (action->actions.size() != 2) {
            context->syntaxError("Coordenates parameters error on CIRCLE");
            return;
          } else if (action->lexeme->value == "COORD") {
            sub_action = action->actions[0];

            result_subtype = expression.evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              expression.addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              cpu.addLdiiHL(0xFCB7);

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = expression.evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              expression.addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              cpu.addLdiiHL(0xFCB9);

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

              has_y_coord = true;
            }

          } else {
            context->syntaxError("Invalid coordenates on CIRCLE");
            return;
          }

        } break;

        // radius
        case 1: {
          result_subtype = expression.evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          has_radius = true;

        } break;

        // color
        case 2: {
          if (has_radius) {
            // push hl
            cpu.addPushHL();
          }

          result_subtype = expression.evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) {
            if (has_radius) {
              cpu.context->code_pointer--;
              cpu.context->code_size--;
            }
            continue;
          }

          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          cpu.addLdAL();

          has_color = true;

        } break;

        // tracing start
        case 3: {
          if (has_radius) {
            if (has_color) {
              // pop hl
              cpu.addPopHL();
            }
            // ld c, l
            cpu.addLdCL();
          }

          // ld hl, BUF
          cpu.addLdHL(def_BUF);
          // push hl
          cpu.addPushHL();
          //   ld (hl), 0x2C  ; comma
          // cpuOpcodeWriter->addLdiHL(0x2C);
          //   inc hl
          // cpuOpcodeWriter->addIncHL();

          if (has_radius) {
            //   ld (hl), 0x0F  ; short interger marker
            cpu.addLdiHL(0x0F);
            //   inc hl
            cpu.addIncHL();
            //   ld (hl), c     ; radius
            cpu.addLdiHLC();
            //   inc hl
            cpu.addIncHL();
          }

          //   ld (hl), 0x2C  ; comma
          cpu.addLdiHL(0x2C);
          //   inc hl
          cpu.addIncHL();

          if (has_color) {
            //   ld (hl), 0x0F  ; short interger marker
            cpu.addLdiHL(0x0F);
            //   inc hl
            cpu.addIncHL();
            //   ld (hl), a     ; color
            cpu.addLdiHLA();
            //   inc hl
            cpu.addIncHL();
          }

          //   ld (hl), 0x2C  ; comma
          cpu.addLdiHL(0x2C);
          //   inc hl
          cpu.addIncHL();

          // push hl
          cpu.addPushHL();

          result_subtype = expression.evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) {
            cpu.context->code_pointer--;
            cpu.context->code_size--;
            continue;
          }

          expression.addCast(result_subtype, Lexeme::subtype_single_decimal);

          // pop de
          optimizer.addByteOptimized(0xD1);

          // call WriteParamBCD             ; b:hl, de -> hl
          cpu.addCall(def_WriteParamBCD);

          has_trace1 = true;

        } break;

        // tracing end
        case 4: {
          //   ld (hl), 0x2C  ; comma
          cpu.addLdiHL(0x2C);
          //   inc hl
          cpu.addIncHL();

          // push hl
          cpu.addPushHL();

          result_subtype = expression.evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) {
            cpu.context->code_pointer--;
            cpu.context->code_size--;
            continue;
          }

          expression.addCast(result_subtype, Lexeme::subtype_single_decimal);

          // pop de
          optimizer.addByteOptimized(0xD1);

          // call WriteParamBCD             ; b:hl, de -> hl
          cpu.addCall(def_WriteParamBCD);

          has_trace2 = true;
        } break;

        // aspect ratio
        case 5: {
          //   ld (hl), 0x2C  ; comma
          cpu.addLdiHL(0x2C);
          //   inc hl
          cpu.addIncHL();

          // push hl
          cpu.addPushHL();

          result_subtype = expression.evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) {
            cpu.context->code_pointer -= 3;
            cpu.context->code_size -= 3;
            continue;
          }

          expression.addCast(result_subtype, Lexeme::subtype_single_decimal);

          // pop de
          optimizer.addByteOptimized(0xD1);

          // call WriteParamBCD             ; b:hl, de -> hl
          cpu.addCall(def_WriteParamBCD);

          has_aspect = true;
        } break;

        default: {
          context->syntaxError("Invalid CIRCLE parameters");
          return;
        }
      }
    }

    if (has_trace1 || has_trace2 || has_aspect) {
      //   ld (hl), 0x00
      cpu.addLdiHL(0x00);
      //   inc hl
      cpu.addIncHL();

      // pop hl
      cpu.addPopHL();

      // ld a, (hl)         ; first character
      cpu.addLdAiHL();
      // ld ix, M5B16       ; rom basic circle without coords
      cpu.addLdIX(0x5B16);
      // call xbasic CIRCLE2 (in: hl = basic line starting on radius parameter)
      cpu.addCall(def_XBASIC_CIRCLE2);

    } else {
      if (has_color && has_radius) {
        // pop hl
        cpu.addPopHL();
      } else if (!has_color) {
        // ld a, (0xF3E9)       ; FORCLR
        cpu.addLdAii(0xF3E9);
      }

      if (!has_radius) {
        // ld hl, 0
        cpu.addLdHL(0x0000);
      }

      if (!has_y_coord) {
      }

      if (!has_x_coord) {
      }

      // call xbasic CIRCLE (in: GRPACX/GRPACY, hl=radius, a=color)
      cpu.addCall(def_XBASIC_CIRCLE);
    }

  } else {
    context->syntaxError("CIRCLE with empty parameters");
  }
}

bool CompilerCircleStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_circle(context);
  return context->compiled;
}
