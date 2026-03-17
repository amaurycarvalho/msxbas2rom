#include "compiler_copy_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"

void CompilerCopyStatementStrategy::cmd_copy(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<ActionNode> action, sub_action;
  shared_ptr<Lexeme> lexeme;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype, state;
  bool has_x0_coord = false, has_x1_coord = false, has_x2_coord = false;
  bool has_y0_coord = false, has_y1_coord = false, has_y2_coord = false;
  bool has_src_page = false, has_dest_page = false, has_operator = false;
  bool has_address_from = false, has_address_to = false;

  if (t) {
    // action = ctx.current_action->actions[0];

    state = 0;

    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];

      switch (state) {
        case 0: {
          if (action->lexeme->value == "SCREEN") {
            cmd_copy_screen(context);
            return;
          } else if (action->lexeme->value == "COORD") {
            if (action->actions.size() != 2) {
              context->syntaxError("Coordenates parameters error on COPY");
              return;
            }

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
            // copy from RAM address to vram
            has_address_from = true;

            if ((lexeme = action->lexeme)) {
              if (lexeme->type == Lexeme::type_identifier && lexeme->isArray) {
                // ld hl, variable
                fixup.addFix(lexeme);
                cpu.addLdHL(0x0000);
                result_subtype = Lexeme::subtype_numeric;
              } else {
                result_subtype = expression.evalExpression(action);
              }
            } else {
              result_subtype = expression.evalExpression(action);
            }

            if (result_subtype == Lexeme::subtype_null ||
                result_subtype == Lexeme::subtype_single_decimal ||
                result_subtype == Lexeme::subtype_double_decimal) {
              context->syntaxError("Invalid address in COPY");
              return;
            }

            // push hl
            cpu.addPushHL();

            state++;
          }

        } break;

        case 1: {
          state++;

          result_subtype = expression.evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          cpu.addLdAL();
          // push af
          cpu.addPushAF();

          has_src_page = true;

        } break;

        case 2: {
          if (action->lexeme->value == "TO_DEST") {
            state++;

            sub_action = action->actions[0];

            result_subtype = expression.evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              expression.addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              cpu.addLdiiHL(0xFCB7);
              // push hl
              cpu.addPushHL();
              has_x2_coord = true;
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
              has_y2_coord = true;
            }

          } else {
            // copy from VRAM to RAM address
            has_address_to = true;

            if ((lexeme = action->lexeme)) {
              if (lexeme->type == Lexeme::type_identifier && lexeme->isArray) {
                // ld hl, variable
                fixup.addFix(lexeme);
                cpu.addLdHL(0x0000);
                result_subtype = Lexeme::subtype_numeric;
              } else {
                result_subtype = expression.evalExpression(action);
              }
            } else {
              result_subtype = expression.evalExpression(action);
            }

            if (result_subtype == Lexeme::subtype_null ||
                result_subtype == Lexeme::subtype_single_decimal ||
                result_subtype == Lexeme::subtype_double_decimal) {
              context->syntaxError("Invalid address in COPY");
              return;
            }

            state = 99;  // exit loop
          }

        } break;

        case 3: {
          state++;

          result_subtype = expression.evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          cpu.addLdAL();
          // push af
          cpu.addPushAF();

          has_dest_page = true;

        } break;

        case 4: {
          state++;

          result_subtype = expression.evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          cpu.addLdAL();
          cpu.addPushAF();

          has_operator = true;

        } break;

        default: {
          context->syntaxError("COPY parameters not supported");
          return;
        }
      }
    }

    if (has_address_from) {
      if (!has_operator) {
        // xor a
        cpu.addXorA();
      } else {
        cpu.addPopAF();
      }
      // ld b, a
      cpu.addLdBA();
      // ld (LOGOP), a
      cpu.addLdiiA(def_LOGOP);

      if (has_dest_page) {
        // pop af
        cpu.addPopAF();
        // ld (ACPAGE), a
        cpu.addLdiiA(def_ACPAGE);
      }

      if (has_y2_coord) {
        // pop hl
        cpu.addPopHL();
      } else {
        // ld hl, (0xFCB9)  ;GRPACY
        cpu.addLdHLii(0xFCB9);
      }
      // ld (DY), hl
      cpu.addLdiiHL(def_DY);

      if (has_x2_coord) {
        // pop hl
        cpu.addPopHL();
      } else {
        // ld hl, (0xFCB7)  ;GRPACX
        cpu.addLdHLii(0xFCB7);
      }
      // ld (DX), hl
      cpu.addLdiiHL(def_DX);

      if (has_src_page) {
        // pop af
        cpu.addPopAF();
        // add a,a
        cpu.addAddA();
        // add a,a
        cpu.addAddA();
        // ld (ARGT), a    ; direction/expansion (0000DDEE)
        cpu.addLdiiA(def_ARGT);
      }

      // pop hl
      cpu.addPopHL();

      // call XBASIC_COPY_FROM
      cpu.addCall(def_XBASIC_COPY_FROM);

    } else if (has_address_to) {
      // ex de,hl      ; address to (hl to de)
      cpu.addExDEHL();

      if (has_src_page) {
        // pop af
        cpu.addPopAF();
        // ld (ACPAGE), a
        cpu.addLdiiA(def_ACPAGE);
      }

      if (has_y1_coord) {
        // pop hl
        cpu.addPopHL();
      } else {
        // ld hl, (0xFCB9)  ;GRPACY
        cpu.addLdHLii(0xFCB9);
      }
      // ld (NY), hl
      cpu.addLdiiHL(def_NY);

      if (has_x1_coord) {
        // pop hl
        cpu.addPopHL();
      } else {
        // ld hl, (0xFCB7)  ;GRPACX
        cpu.addLdHLii(0xFCB7);
      }
      // ld (NX), hl
      cpu.addLdiiHL(def_NX);

      if (has_y0_coord) {
        // pop hl
        cpu.addPopHL();
      } else {
        // ld hl, (0xFCB9)  ;GRPACY
        cpu.addLdHLii(0xFCB9);
      }
      // ld (SY), hl
      cpu.addLdiiHL(def_SY);

      if (has_x0_coord) {
        // pop hl
        cpu.addPopHL();
      } else {
        // ld hl, (0xFCB7)  ;GRPACX
        cpu.addLdHLii(0xFCB7);
      }
      // ld (SX), hl
      cpu.addLdiiHL(def_SX);

      // ex de,hl
      cpu.addExDEHL();

      // call XBASIC_COPY_TO
      cpu.addCall(def_XBASIC_COPY_TO);

    } else {
      if (!has_operator) {
        // ld b, 0
        cpu.addLdB(0x00);
      } else {
        cpu.addPopBC();
      }

      if (has_dest_page) {
        // pop af
        cpu.addPopAF();
      } else {
        // ld a, (ACPAGE)
        cpu.addLdAii(def_ACPAGE);
      }
      // ld (0xFC19), a
      cpu.addLdiiA(0xFC19);

      if (has_y2_coord) {
        // pop hl
        cpu.addPopHL();
        // ld (0xFCB9), hl  ;GRPACY
        cpu.addLdiiHL(0xFCB9);
      }

      if (has_x2_coord) {
        // pop hl
        cpu.addPopHL();
        // ld (0xFCB7), hl  ;GRPACX
        cpu.addLdiiHL(0xFCB7);
      }

      if (has_src_page) {
        // pop af
        cpu.addPopAF();
      } else {
        // ld a, (ACPAGE)
        cpu.addLdAii(def_ACPAGE);
      }
      // ld (0xFC18), a
      cpu.addLdiiA(0xFC18);

      if (has_y1_coord) {
        // pop iy
        cpu.addPopIY();
      } else {
        // ld iy, (0xFCB9)  ;GRPACY
        cpu.addLdIYii(0xFCB9);
      }

      if (has_x1_coord) {
        // pop ix
        cpu.addPopIX();
      } else {
        // ld ix, (0xFCB7)  ;GRPACX
        cpu.addLdIXii(0xFCB7);
      }

      if (has_y0_coord) {
        // pop hl
        cpu.addPopHL();
      } else {
        // ld hl, (0xFCB9)  ;GRPACY
        cpu.addLdHLii(0xFCB9);
      }

      if (has_x0_coord) {
        // pop de
        cpu.addPopDE();
      } else {
        // ld de, (0xFCB7)  ;GRPACX
        cpu.addLdDEii(0xFCB7);
      }

      // call COPY    ; in: de=x0, hl=y0, ix=x1, iy=y1, 0xFC18=srcpg, 0xFCB7=x2,
      // 0xFCB9=y2, 0xFC19=destpg, b=operator
      cpu.addCall(def_XBASIC_COPY);
    }

  } else {
    context->syntaxError("COPY with empty parameters");
  }
}

void CompilerCopyStatementStrategy::cmd_copy_screen(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  int t = context->current_action->actions.size();

  if (t > 1) {
    context->syntaxError("Invalid COPY SCREEN parameters");
  } else {
    if (t == 0) {
      // xor a
    } else if (t == 1) {
      shared_ptr<ActionNode> action = context->current_action->actions[0];
      shared_ptr<ActionNode> sub_action;
      int result_subtype;

      sub_action = action->actions[0];
      result_subtype = expression.evalExpression(sub_action);

      expression.addCast(result_subtype, Lexeme::subtype_numeric);

      // ld a, l
      cpu.addLdAL();
    }

    // and 1
    cpu.addAnd(0x01);
    // inc a
    cpu.addIncA();
    // or 0x10
    cpu.addOr(0x10);
    // ld hl, BUF
    cpu.addLdHL(def_BUF);
    // push hl
    cpu.addPushHL();
    //   ld (hl), 0xC5  ; SCREEN token
    cpu.addLdiHL(0xC5);
    //   inc hl
    cpu.addIncHL();
    //   ld (hl), a
    cpu.addLdiHLA();
    //   inc hl
    cpu.addIncHL();
    //   xor a
    cpu.addXorA();
    //   ld (hl), a
    cpu.addLdiHLA();
    //   inc hl
    cpu.addIncHL();
    //   ld (hl), a
    cpu.addLdiHLA();
    // pop hl
    cpu.addPopHL();

    // ld a, (VERSION)
    cpu.addLdAii(def_VERSION);
    // and a
    cpu.addAndA();
    // jr z, skip
    cpu.addJrZ(9);

    //   ld a, (hl)       ; first character
    cpu.addLdAiHL();
    //   ld ix, (COPY)    ; COPY
    cpu.addLdIXii(def_COPY_STMT);
    //   call CALBAS
    cpu.addCall(def_CALBAS);
    //   ei
    cpu.addEI();

    // skip:
  }

  return;
}

bool CompilerCopyStatementStrategy::execute(CompilerContext* context) {
  cmd_copy(context);
  return context->compiled;
}
