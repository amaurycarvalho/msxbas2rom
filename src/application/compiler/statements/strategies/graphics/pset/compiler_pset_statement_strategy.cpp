#include "compiler_pset_statement_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

void CompilerPsetStatementStrategy::cmd_pset(
    shared_ptr<CompilerContext> context, bool forecolor) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<ActionNode> action, sub_action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;
  bool has_x_coord = false, has_y_coord = false, has_color = false,
       has_operator = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];

      switch (i) {
        case 0: {
          if (action->actions.size() != 2) {
            if (forecolor)
              context->syntaxError("Coordenates parameters error on PSET");
            else
              context->syntaxError("Coordenates parameters error on PRESET");
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
            if (forecolor)
              context->syntaxError("Invalid coordenates on PSET");
            else
              context->syntaxError("Invalid coordenates on PRESET");
            return;
          }

        } break;

        case 1: {
          result_subtype = expression.evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          cpu.addLdAL();

          // push af       ; save color
          cpu.addPushAF();

          has_color = true;

        } break;

        case 2: {
          result_subtype = expression.evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          cpu.addLdAL();

          // ld (LOGOPR), a          ; save new logical operator to basic
          // interpreter
          cpu.addLdiiA(def_LOGOPR);

          has_operator = true;

        } break;

        default: {
          if (forecolor)
            context->syntaxError("PSET parameters not supported");
          else
            context->syntaxError("PRESET parameters not supported");
          return;
        }
      }
    }

    if (!has_operator) {
      // ld a, (LOGOPR)      ; get default operator from basic interpreter
      cpu.addLdAii(def_LOGOPR);
    }

    // ld b, a      ; get operator
    cpu.addLdBA();

    if (has_color) {
      // pop af       ; color
      cpu.addPopAF();
    } else {
      if (forecolor) {
        // ld a, (0xF3E9)       ; FORCLR
        cpu.addLdAii(def_FORCLR);
      } else {
        // ld a, (0xF3EA)       ; BAKCLR
        cpu.addLdAii(def_BAKCLR);
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

    // call 0x6F71   ; xbasic PSET (in: hl=y, de=x, a=color, b=operator)
    cpu.addCall(def_XBASIC_PSET);

  } else {
    if (forecolor)
      context->syntaxError("PSET with empty parameters");
    else
      context->syntaxError("PRESET with empty parameters");
  }
}

bool CompilerPsetStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_pset(context, true);
  return context->compiled;
}
