#include "compiler_put_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

void CompilerPutStatementStrategy::cmd_put(CompilerContext* context) {
  shared_ptr<ActionNode> action;
  shared_ptr<Lexeme> lexeme;
  unsigned int t = context->current_action->actions.size();

  if (t) {
    action = context->current_action->actions[0];
    lexeme = action->lexeme;

    if (lexeme->type == Lexeme::type_keyword && lexeme->value == "SPRITE") {
      context->current_action = action;
      cmd_put_sprite(context);
    } else if (lexeme->type == Lexeme::type_keyword &&
               lexeme->value == "TILE") {
      context->current_action = action;
      cmd_put_tile(context);
    } else {
      context->syntaxError("Invalid PUT statement");
    }

  } else {
    context->syntaxError("Empty PUT statement");
  }
}

void CompilerPutStatementStrategy::cmd_put_sprite(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<ActionNode> action, sub_action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype, parm_flag = 0;
  bool has_sprite = false, has_x_coord = false, has_y_coord = false,
       has_color = false, has_pattern = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];

      switch (i) {
        case 0: {
          result_subtype = expression.evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          cpu.addLdAL();
          // push af
          cpu.addPushAF();

          has_sprite = true;

        } break;

        case 1: {
          if (action->actions.size() != 2) {
            context->syntaxError("Coordenates parameters error on PUT SPRITE");
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
            context->syntaxError("Invalid coordenates on PUT SPRITE");
            return;
          }

        } break;

        case 2: {
          result_subtype = expression.evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          cpu.addLdAL();
          // push af
          cpu.addPushAF();

          has_color = true;

        } break;

        case 3: {
          result_subtype = expression.evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          cpu.addLdAL();
          // push af
          cpu.addPushAF();

          has_pattern = true;

        } break;

        default: {
          context->syntaxError("PUT SPRITE parameters not supported");
          return;
        }
      }
    }

    if (has_pattern) {
      // pop hl
      cpu.addPopHL();
      parm_flag |= 0x20;
    } else {
      // ld h, 0
      // addLdHn(0x00);
    }

    if (has_color) {
      // pop de
      cpu.addPopDE();
      parm_flag |= 0x40;
    } else {
      // ld d, 0
      // cpuOpcodeWriter->addWord(0x16, 0x00);
    }

    if (has_y_coord) {
      // pop iy
      cpu.addPopIY();
      parm_flag |= 0x80;
    } else {
      // ld iy, 0x0000
      // cpuOpcodeWriter->addLdIYii(0x0000);
    }

    if (has_x_coord) {
      // pop ix
      cpu.addPopIX();
      parm_flag |= 0x80;
    } else {
      // ld ix, 0x0000
      // addLdIXnn(0x0000);
    }

    if (has_sprite) {
      // pop af
      cpu.addPopAF();
    } else {
      // xor a
      cpu.addXorA();
    }

    // ld b, parameters flag
    cpu.addLdB(parm_flag);

    // call xbasic PUT SPRITE (in: ix=x, iy=y, d=color, a=sprite number,
    // h=pattern number, b=parameters flag (b11100000)
    cpu.addCall(def_XBASIC_PUT_SPRITE);

  } else {
    context->syntaxError("PUT SPRITE with empty parameters");
  }
}

void CompilerPutStatementStrategy::cmd_put_tile(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<ActionNode> action, sub_action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;
  bool has_tile = false, has_x_coord = false, has_y_coord = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];

      switch (i) {
        case 0: {
          result_subtype = expression.evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          cpu.addLdAL();
          // push af
          cpu.addPushAF();

          has_tile = true;

        } break;

        case 1: {
          if (action->actions.size() != 2) {
            context->syntaxError("Coordenates parameters error on PUT TILE");
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
            context->syntaxError("Invalid coordenates on PUT TILE");
            return;
          }

        } break;

        default: {
          context->syntaxError("PUT TILE parameters not supported");
          return;
        }
      }
    }

    if (has_y_coord) {
      // pop hl
      cpu.addPopHL();
      // inc l        ; y coord (1 based for bios)
      cpu.addIncL();
    } else {
      // ld a, (CSRY)
      cpu.addLdAii(def_CSRY);
      // ld l, a
      cpu.addLdLA();
    }

    if (has_x_coord) {
      // pop bc
      cpu.addPopBC();
      // ld h, c
      cpu.addLdHC();
      // inc h        ; x coord (1 based for bios)
      cpu.addIncH();
    } else {
      // ld a, (CSRX)
      cpu.addLdAii(def_CSRX);
      // ld h, a
      cpu.addLdHA();
    }

    //   call TileAddress   ; in hl=xy, out: hl
    cpu.addCall(def_tileAddress);

    if (has_tile) {
      // pop af
      cpu.addPopAF();
    } else {
      // xor a
      cpu.addXorA();
    }

    // call 0x70b5                  ; xbasic VPOKE (in: hl=address, a=byte)
    cpu.addCall(def_XBASIC_VPOKE);

  } else {
    context->syntaxError("PUT TILE with empty parameters");
  }
}

bool CompilerPutStatementStrategy::execute(CompilerContext* context) {
  cmd_put(context);
  return context->compiled;
}
