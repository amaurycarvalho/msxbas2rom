#include "compiler_cmd_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

void CompilerCmdStatementStrategy::cmd_cmd(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode *action, *sub_action1, *sub_action2, *sub_action3;
  Lexeme* lexeme;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->value == "RUNASM") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);
          // call cmd_runasm
          cpu.addCall(def_cmd_runasm);

        } else {
          context->syntaxError("CMD RUNASM syntax error");
        }

      } else if (lexeme->value == "RUNBAS") {
        context->syntaxError("CMD RUNBAS not implemented yet");

      } else if (lexeme->value == "WRTVRAM") {
        if (action->actions.size() == 2) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);

          sub_action2 = action->actions[1];
          result_subtype = expression.evalExpression(sub_action2);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG), hl
          cpu.addLdiiHL(def_ARG);

          // call cmd_runasm
          cpu.addCall(def_cmd_wrtvram);

        } else {
          context->syntaxError("CMD WRTVRAM syntax error");
        }

      } else if (lexeme->value == "WRTFNT") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);
          // call cmd_wrtfnt
          cpu.addCall(def_cmd_wrtfnt);

        } else {
          context->syntaxError("CMD WRTFNT syntax error");
        }

      } else if (lexeme->value == "WRTCHR") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);
          // call cmd_wrtchr
          cpu.addCall(def_cmd_wrtchr);

        } else {
          context->syntaxError("CMD WRTCHR syntax error");
        }

      } else if (lexeme->value == "WRTCLR") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);
          // call cmd_wrtclr
          cpu.addCall(def_cmd_wrtclr);

        } else {
          context->syntaxError("CMD WRTCLR syntax error");
        }

      } else if (lexeme->value == "WRTSCR") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);
          // call cmd_wrtscr
          cpu.addCall(def_cmd_wrtscr);

        } else {
          context->syntaxError("CMD WRTSCR syntax error");
        }

      } else if (lexeme->value == "WRTSPR") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);
          // call cmd_wrtspr                    ; tiny sprite loader
          cpu.addCall(def_cmd_wrtspr);

        } else {
          context->syntaxError("CMD WRTSPR syntax error");
        }

      } else if (lexeme->value == "WRTSPRPAT") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);
          // call cmd_wrtsprpat
          cpu.addCall(def_cmd_wrtsprpat);

        } else {
          context->syntaxError("CMD WRTSPRPAT syntax error");
        }

      } else if (lexeme->value == "WRTSPRCLR") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);
          // call cmd_wrtsprclr
          cpu.addCall(def_cmd_wrtsprclr);

        } else {
          context->syntaxError("CMD WRTSPRCLR syntax error");
        }

      } else if (lexeme->value == "WRTSPRATR") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);
          // call cmd_wrtspratr
          cpu.addCall(def_cmd_wrtspratr);

        } else {
          context->syntaxError("CMD WRTSPRATR syntax error");
        }

      } else if (lexeme->value == "RAMTOVRAM") {
        if (action->actions.size() == 3) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);

          sub_action2 = action->actions[1];
          result_subtype = expression.evalExpression(sub_action2);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG), hl
          cpu.addLdiiHL(def_ARG);

          sub_action3 = action->actions[2];
          result_subtype = expression.evalExpression(sub_action3);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG2), hl
          cpu.addLdiiHL(def_ARG2);

          // call cmd_ramtovram
          cpu.addCall(def_cmd_ramtovram);

        } else {
          context->syntaxError("CMD RAMTOVRAM syntax error");
        }

      } else if (lexeme->value == "VRAMTORAM") {
        if (action->actions.size() == 3) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);

          sub_action2 = action->actions[1];
          result_subtype = expression.evalExpression(sub_action2);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG), hl
          cpu.addLdiiHL(def_ARG);

          sub_action3 = action->actions[2];
          result_subtype = expression.evalExpression(sub_action3);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG2), hl
          cpu.addLdiiHL(def_ARG2);

          // call cmd_vramtoram
          cpu.addCall(def_cmd_vramtoram);

        } else {
          context->syntaxError("CMD VRAMTORAM syntax error");
        }

      } else if (lexeme->value == "DISSCR") {
        // call cmd_disscr
        cpu.addCall(def_cmd_disscr);

      } else if (lexeme->value == "ENASCR") {
        // call cmd_enascr
        cpu.addCall(def_cmd_enascr);

      } else if (lexeme->value == "KEYCLKOFF") {
        // call cmd_keyclkoff
        cpu.addCall(def_cmd_keyclkoff);

      } else if (lexeme->value == "MUTE") {
        // call cmd_mute
        cpu.addCall(def_cmd_mute);

      } else if (lexeme->value == "PLAY") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);

          // xor a
          cpu.addXorA();
          // ld (ARG), a
          cpu.addLdiiA(def_ARG);

          // call cmd_play
          cpu.addCall(def_cmd_play);

        } else if (action->actions.size() == 2) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);

          sub_action2 = action->actions[1];
          result_subtype = expression.evalExpression(sub_action2);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG), hl
          cpu.addLdiiHL(def_ARG);

          // call cmd_play
          cpu.addCall(def_cmd_play);

        } else {
          context->syntaxError("CMD PLAY syntax error");
        }

      } else if (lexeme->value == "DRAW") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);
          // call cmd_draw
          cpu.addCall(def_cmd_draw);

        } else {
          context->syntaxError("CMD DRAW syntax error");
        }

      } else if (lexeme->value == "PT3LOAD") {
        context->pt3 = true;
        context->syntaxError("Arkos Tracker not permitted with PT3 player");

      } else if (lexeme->value == "PT3PLAY") {
        context->pt3 = true;
        context->syntaxError("Arkos Tracker not permitted with PT3 player");

      } else if (lexeme->value == "PT3MUTE") {
        context->pt3 = true;
        context->syntaxError("Arkos Tracker not permitted with PT3 player");

      } else if (lexeme->value == "PT3LOOP") {
        context->pt3 = true;
        context->syntaxError("Arkos Tracker not permitted with PT3 player");

      } else if (lexeme->value == "PT3REPLAY") {
        context->pt3 = true;
        context->syntaxError("Arkos Tracker not permitted with PT3 player");

      } else if (lexeme->value == "PLYLOAD") {
        context->akm = true;

        if (action->actions.size()) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);

          if (action->actions.size() > 1) {
            sub_action2 = action->actions[1];
            result_subtype = expression.evalExpression(sub_action2);
            expression.addCast(result_subtype, Lexeme::subtype_numeric);
          }

          // ld (ARG), hl
          cpu.addLdiiHL(def_ARG);

        } else {
          context->syntaxError("CMD PLYLOAD syntax error");
        }

        // call cmd_plyload
        cpu.addCall(def_cmd_plyload);

      } else if (lexeme->value == "PLYSONG") {
        context->akm = true;

        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);

        } else {
          context->syntaxError("CMD PLYSONG syntax error");
        }

        // call cmd_plysong
        cpu.addCall(def_cmd_plysong);

      } else if (lexeme->value == "PLYPLAY") {
        context->akm = true;

        // call cmd_plyplay
        cpu.addCall(def_cmd_plyplay);

      } else if (lexeme->value == "PLYMUTE") {
        context->akm = true;

        // call cmd_akmmute
        cpu.addCall(def_cmd_plymute);

      } else if (lexeme->value == "PLYLOOP") {
        context->akm = true;

        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);
          // call cmd_plyloop
          cpu.addCall(def_cmd_plyloop);

        } else {
          context->syntaxError("CMD PLYLOOP syntax error");
        }

      } else if (lexeme->value == "PLYREPLAY") {
        context->akm = true;

        // call cmd_plyreplay
        cpu.addCall(def_cmd_plyreplay);

      } else if (lexeme->value == "PLYSOUND") {
        if (action->actions.size()) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);

          if (action->actions.size() > 1) {
            sub_action2 = action->actions[1];
            result_subtype = expression.evalExpression(sub_action2);
            expression.addCast(result_subtype, Lexeme::subtype_numeric);
          } else {
            // ld hl, 0
            cpu.addLdHL(0x0000);
          }

          // ld (ARG), hl
          cpu.addLdiiHL(def_ARG);

          if (action->actions.size() > 2) {
            sub_action2 = action->actions[1];
            result_subtype = expression.evalExpression(sub_action2);
            expression.addCast(result_subtype, Lexeme::subtype_numeric);
          } else if (action->actions.size() > 1) {
            // ld hl, 0
            cpu.addLdHL(0x0000);
          }

          // ld (ARG+2), hl
          cpu.addLdiiHL(def_ARG + 2);

        } else {
          context->syntaxError("CMD PLYSOUND syntax error");
        }

        // call cmd_plysound
        cpu.addCall(def_cmd_plysound);

      } else if (lexeme->value == "MTF") {
        if (!action->actions.size()) {
          context->syntaxError("CMD MTF parameters is missing");
        } else if (action->actions.size() > 4) {
          context->syntaxError("CMD MTF syntax error");
        } else {
          // resource number parameter
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          if (action->actions.size() > 1) {
            cpu.addPushHL();

            // map operation parameter
            sub_action2 = action->actions[1];
            result_subtype = expression.evalExpression(sub_action2);
            expression.addCast(result_subtype, Lexeme::subtype_numeric);

            if (action->actions.size() > 2) {
              cpu.addLdHL();
              cpu.addPushHL();

              // col/x parameter
              sub_action2 = action->actions[2];
              result_subtype = expression.evalExpression(sub_action2);
              expression.addCast(result_subtype, Lexeme::subtype_numeric);

              if (action->actions.size() > 3) {
                cpu.addPushHL();

                // row/y parameter
                sub_action2 = action->actions[3];
                result_subtype = expression.evalExpression(sub_action2);
                expression.addCast(result_subtype, Lexeme::subtype_numeric);

                cpu.addLdCL();
                cpu.addLdBH();   //! row/y
                cpu.addPopDE();  //! col/x
                cpu.addPopAF();  //! map operation
                cpu.addPopHL();  //! resource number
              } else {
                cpu.addExDEHL();  //! col/x
                cpu.addLdBC(0);   //! row/y
                cpu.addPopAF();   //! map operation
                cpu.addPopHL();   //! resource number
              }
            } else {
              cpu.addXorA();
              cpu.addLdEA();
              cpu.addLdDA();  //! col/x
              cpu.addLdCA();
              cpu.addLdBA();   //! row/y
              cpu.addLdAL();   //! map operation
              cpu.addPopHL();  //! resource number
            }
          } else {
            cpu.addXorA();  //! map operation
            cpu.addLdEA();
            cpu.addLdDA();  //! col/x
            cpu.addLdCA();
            cpu.addLdBA();  //! row/y
          }

          // hl = resource number
          // de = col/x position
          // bc = row/y position
          // a = map operation
          // call cmd_mtf
          cpu.addCall(def_cmd_mtf);
        }
      } else if (lexeme->value == "SETFNT") {
        context->font = true;

        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);

          // ld a, 0xff                ; it means all screen banks
          cpu.addLdA(0xFF);
          // ld (ARG), a
          cpu.addLdiiA(def_ARG);

          // call cmd_setfnt
          cpu.addCall(def_cmd_setfnt);

        } else if (action->actions.size() == 2) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);

          sub_action2 = action->actions[1];
          result_subtype = expression.evalExpression(sub_action2);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG), hl             ; screen font bank number
          cpu.addLdiiHL(def_ARG);

          // call cmd_setfnt
          cpu.addCall(def_cmd_setfnt);

        } else {
          context->syntaxError("CMD SETFNT syntax error");
        }
      } else if (lexeme->value == "UPDFNTCLR") {
        // call cmd_disscr
        cpu.addCall(def_cmd_updfntclr);
      } else if (lexeme->value == "CLRSCR") {
        // call cmd_clrscr
        cpu.addCall(def_cmd_clrscr);
      } else if (lexeme->value == "RAMTORAM") {
        if (action->actions.size() == 3) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);

          sub_action2 = action->actions[1];
          result_subtype = expression.evalExpression(sub_action2);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG), hl
          cpu.addLdiiHL(def_ARG);

          sub_action3 = action->actions[2];
          result_subtype = expression.evalExpression(sub_action3);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG2), hl
          cpu.addLdiiHL(def_ARG2);

          // call cmd_ramtoram
          cpu.addCall(def_cmd_ramtoram);

        } else {
          context->syntaxError("CMD RAMTORAM syntax error");
        }
      } else if (lexeme->value == "RSCTORAM") {
        if (action->actions.size() == 2 || action->actions.size() == 3) {
          sub_action1 = action->actions[0];  // resource number
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // push hl
          cpu.addPushHL();

          sub_action2 = action->actions[1];  // ram dest address
          result_subtype = expression.evalExpression(sub_action2);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          if (action->actions.size() == 2) {
            // ex de, hl
            cpu.addExDEHL();
            // xor a   ; no pletter
            cpu.addXorA();
          } else {
            // push hl
            cpu.addPushHL();

            sub_action3 = action->actions[2];  // pletter? 0=no, 1=yes
            result_subtype = expression.evalExpression(sub_action3);
            expression.addCast(result_subtype, Lexeme::subtype_numeric);

            // ld a, l
            cpu.addLdAL();

            // pop de
            cpu.addPopDE();
          }

          // pop hl
          cpu.addPopHL();

          // call cmd_rsctoram
          cpu.addCall(def_cmd_rsctoram);

        } else {
          context->syntaxError("CMD RSCTORAM syntax error");
        }
      } else if (lexeme->value == "CLRKEY") {
        // call cmd_clrkey
        cpu.addCall(def_cmd_clrkey);
      } else if (lexeme->value == "CLIP") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          cpu.addLdAL();
          // ld hl, 0xfafc
          cpu.addLdHL(0xfafc);
          // res 4, (hl)
          cpu.addWord(0xCB, 0xA6);
          // bit 0, a
          cpu.addWord(0xCB, 0x47);
          // jr z, +3
          cpu.addJrZ(0x02);
          //   set 4, (hl)
          cpu.addWord(0xCB, 0xE6);

        } else {
          context->syntaxError("CMD CLIP syntax error");
        }
      } else if (lexeme->value == "TURBO") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);
          // call cmd_turbo
          cpu.addCall(def_cmd_turbo);

        } else {
          context->syntaxError("CMD TURBO syntax error");
        }
      } else if (lexeme->value == "RESTORE") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          cpu.addLdiiHL(def_DAC);

          // call cmd_restore
          cpu.addCall(
              def_cmd_restore);  // MSXBAS2ROM resource RESTORE statement

        } else {
          context->syntaxError("CMD RESTORE syntax error");
        }
      } else if (lexeme->value == "PAGE") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // ld de, 0                 ; delay = stop
          cpu.addLdDE(0);

          // push de
          cpu.addPushDE();

          // pop bc
          cpu.addPopBC();

        } else if (action->actions.size() == 2) {
          sub_action1 = action->actions[1];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // push hl
          cpu.addPushHL();
          // push hl
          cpu.addPushHL();

          sub_action2 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action2);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // pop de
          cpu.addPopDE();

          // pop bc
          cpu.addPopBC();

        } else if (action->actions.size() == 3) {
          sub_action1 = action->actions[2];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // push hl
          cpu.addPushHL();

          sub_action1 = action->actions[1];
          result_subtype = expression.evalExpression(sub_action1);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // push hl
          cpu.addPushHL();

          sub_action2 = action->actions[0];
          result_subtype = expression.evalExpression(sub_action2);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          // pop de
          cpu.addPopDE();

          // pop bc
          cpu.addPopBC();

        } else {
          context->syntaxError("CMD PAGE syntax error");
        }

        // call cmd_page (l = mode, e = delay #1, c = delay #2)
        cpu.addCall(def_cmd_page);
      } else {
        context->syntaxError("CMD statement invalid");
        return;
      }
    }
  } else {
    context->syntaxError("CMD with empty parameters");
  }
}

bool CompilerCmdStatementStrategy::execute(CompilerContext* context) {
  cmd_cmd(context);
  return context->compiled;
}
