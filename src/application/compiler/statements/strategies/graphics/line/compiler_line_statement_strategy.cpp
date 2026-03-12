#include "compiler_line_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "compiler_input_statement_strategy.h"

void CompilerLineStatementStrategy::cmd_line(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  Lexeme* lexeme;
  ActionNode *action, *sub_action;
  unsigned int i, t = context->current_action->actions.size();
  int line_type = 0, result_subtype, state;
  bool has_x0_coord = false, has_x1_coord = false, has_y0_coord = false,
       has_y1_coord = false;
  bool has_color = false, has_line_type = false, has_operator = false;

  if (t) {
    action = context->current_action->actions[0];
    lexeme = action->lexeme;
    if (lexeme->type == Lexeme::type_keyword && lexeme->value == "INPUT") {
      CompilerInputStatementStrategy inputStmt;
      context->current_action = action;
      if (!inputStmt.executeLineInput(context)) {
        if (!context->containErrors())
          context->syntaxError("Error on LINE INPUT");
      }
      return;
    }

    state = 0;

    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];

      switch (state) {
        case 0: {
          if (action->actions.size() != 2) {
            context->syntaxError("Coordenates parameters error on LINE");
            return;
          } else if (action->lexeme->value == "COORD") {
            sub_action = action->actions[0];

            result_subtype = expression.evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
              // ld hl, (0xFCB7)  ;GRPACX
              cpu.addLdHLii(0xFCB7);
              // push hl
              cpu.addPushHL();
              has_x0_coord = true;
            } else {
              expression.addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              cpu.addLdiiHL(0xFCB7);
              // push hl
              cpu.addPushHL();
              has_x0_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = expression.evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
              // ld hl, (0xFCB9)  ;GRPACY
              cpu.addLdHLii(0xFCB9);
              // push hl
              cpu.addPushHL();
              has_y0_coord = true;
            } else {
              expression.addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              cpu.addLdiiHL(0xFCB9);
              // push hl
              cpu.addPushHL();
              has_y0_coord = true;
            }

          } else if (action->lexeme->value == "STEP") {
            sub_action = action->actions[0];

            result_subtype = expression.evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
              // ld hl, (0xFCB7)  ;GRPACX
              cpu.addLdHLii(0xFCB7);
              // push hl
              cpu.addPushHL();
              has_x0_coord = true;
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
              has_x0_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = expression.evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
              // ld hl, (0xFCB9)  ;GRPACY
              cpu.addLdHLii(0xFCB9);
              // push hl
              cpu.addPushHL();
              has_y0_coord = true;
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
              has_y0_coord = true;
            }

          } else if (action->lexeme->value == "TO_COORD") {
            state++;

            if (!has_x0_coord) {
              // ex de,hl
              cpu.addExDEHL();
              // ld hl, (0xFCB7)  ;GRPACX
              cpu.addLdHLii(0xFCB7);
              // push hl
              cpu.addPushHL();
              // ld hl, (0xFCB9)  ;GRPACY
              cpu.addLdHLii(0xFCB9);
              // push hl
              cpu.addPushHL();
              // ex de,hl
              cpu.addExDEHL();
              has_x0_coord = true;
              has_y0_coord = true;
            }

            sub_action = action->actions[0];

            result_subtype = expression.evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              expression.addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              cpu.addLdiiHL(0xFCB7);
              // push hl
              cpu.addPushHL();
              has_x1_coord = true;
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
              has_y1_coord = true;
            }

          } else if (action->lexeme->value == "TO_STEP") {
            state++;

            if (!has_x0_coord) {
              // ex de,hl
              cpu.addExDEHL();
              // ld hl, (0xFCB7)  ;GRPACX
              cpu.addLdHLii(0xFCB7);
              // push hl
              cpu.addPushHL();
              // ld hl, (0xFCB9)  ;GRPACY
              cpu.addLdHLii(0xFCB9);
              // push hl
              cpu.addPushHL();
              // ex de,hl
              cpu.addExDEHL();
              has_x0_coord = true;
              has_y0_coord = true;
            }

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
              has_x1_coord = true;
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
              has_y1_coord = true;
            }

          } else {
            context->syntaxError("Invalid coordenates on LINE");
            return;
          }

        } break;

        case 1: {
          state++;

          result_subtype = expression.evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          cpu.addLdAL();

          has_color = true;

        } break;

        case 2: {
          state++;

          lexeme = action->lexeme;
          if (lexeme->subtype == Lexeme::subtype_null) continue;

          if (lexeme->type == Lexeme::type_literal) {
            try {
              line_type = stoi(lexeme->value);
            } catch (exception& e) {
              printf("Warning: error while converting numeric constant %s\n",
                     lexeme->value.c_str());
              line_type = 0;
            }
          } else {
            context->syntaxError("Invalid shape parameter");
          }

          has_line_type = true;

        } break;

        case 3: {
          state++;

          result_subtype = expression.evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld b, l
          cpu.addLdBL();

          has_operator = true;

        } break;

        default: {
          context->syntaxError("LINE parameters not supported");
          return;
        }
      }
    }

    if (!has_color) {
      // ld a, (0xF3E9)       ; FORCLR
      cpu.addLdAii(0xF3E9);
    }

    if (!has_operator) {
      // ld b, 0
      cpu.addLdB(0x00);
    }

    if (has_y1_coord) {
      // pop hl
      cpu.addPopHL();
    } else {
      // ld hl, (0xFCB9)  ;GRPACY
      cpu.addLdHLii(0xFCB9);
    }

    if (has_x1_coord) {
      // pop de
      cpu.addPopDE();
    } else {
      // ld de, (0xFCB7)  ;GRPACX
      cpu.addLdDEii(0xFCB7);
    }

    if (has_y0_coord) {
      // pop iy
      cpu.addPopIY();
    } else {
      // ld iy, (0xFCB9)  ;GRPACY
      cpu.addLdIYii(0xFCB9);
    }

    if (has_x0_coord) {
      // pop ix
      cpu.addPopIX();
    } else {
      // ld ix, (0xFCB7)  ;GRPACX
      cpu.addLdIXii(0xFCB7);
    }

    if (line_type == 0 || !has_line_type) {
      // call 0x6DA7   ; xbasic LINE (in: ix=x0, iy=y0, de=x1, hl=y1, a=color,
      // b=operator)
      cpu.addCall(def_XBASIC_LINE);
    } else if (line_type == 1) {
      // call 0x6D49   ; xbasic BOX (in: ix=x0, iy=y0, de=x1, hl=y1, a=color,
      // b=operator)
      cpu.addCall(def_XBASIC_BOX);
    } else {
      // call 0x6E27   ; xbasic BOX FILLED (in: ix=x0, iy=y0, de=x1, hl=y1,
      // a=color, b=operator)
      cpu.addCall(def_XBASIC_BOXF);
    }

  } else {
    context->syntaxError("LINE with empty parameters");
  }
}

bool CompilerLineStatementStrategy::execute(CompilerContext* context) {
  cmd_line(context);
  return context->compiled;
}
