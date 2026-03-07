/***
 * @file compiler_statements.cpp
 * @brief Compiler statement emission and related helper routines
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_statement_emitter.h"

#include "compiler_code_helper.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_evaluator.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "compiler_symbol_resolver.h"
#include "compiler_variable_emitter.h"

void CompilerStatementEmitter::cmd_start() {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& parser = *context->parser;
  auto& opts = *context->opts;

  // ld (SAVSTK), sp
  cpu.addLdiiSP(0xf6b1);

  // ld a, (SLTSTR)     ; start slot
  cpu.addLdAii(def_SLTSTR);

  // ld h, 0x40
  cpu.addLdH(0x40);

  // call ENASLT        ; enable xbasic page
  cpu.addCall(def_ENASLT);

  // ld hl, HEAP START ADDRESS
  fixup.addFix(context->heap_mark.get());
  cpu.addLdHL(0x0000);

  // ld de, TEMPORARY STRING START ADDRESS
  fixup.addFix(context->temp_str_mark.get());
  cpu.addLdDE(0x0000);

  if (parser.getHasFont()) {
    // ld ix, FONT BUFFER START ADDRESS
    cpu.addByte(0xDD);
    fixup.addFix(context->heap_mark.get())->step = -def_RAM_BUFSIZ;
    cpu.addLdHL(0x0000);
  } else {
    // push hl
    cpu.addPushHL();
    // pop ix
    cpu.addPopIX();
  }

  // call XBASIC INIT                  ; hl=heap start address, de=temporary
  // string start address, bc=data address, ix=font address, a=data segment
  cpu.addCall(def_XBASIC_INIT);

  if (parser.getHasTraps()) {
    if (opts.megaROM) {
      // ld a, 0xFF
      cpu.addLdA(0xFF);
    } else {
      // xor a
      cpu.addXorA();
    }
    // ld (0xFC82), a          ; start of TRPTBL reserved area (megaROM flag to
    // traps)
    cpu.addLdiiA(def_MR_TRAP_FLAG);
  }

  if (opts.megaROM) {
    // ld a, 2
    cpu.addLdA(0x02);
    // call MR_CHANGE_SGM
    cpu.addCall(def_MR_CHANGE_SGM);
  }

  if (parser.getHasData() || parser.getHasIData()) {
    // ld hl, data resource number
    cpu.addLdHL(parser.getResourceCount());
    // ld (DAC), hl
    cpu.addLdiiHL(def_DAC);
    // call cmd_restore
    cpu.addCall(def_cmd_restore);  // MSXBAS2ROM resource RESTORE statement
  }

  if (parser.getHasAkm()) {
    // initialize akm player
    cpu.addCall(def_player_initialize);
  }

  // ei
  cpu.addEI();
}

void CompilerStatementEmitter::cmd_print() {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
  Lexeme *lexeme, *last_lexeme = 0;
  ActionNode *action, *subaction;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;
  bool redirected = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;
      last_lexeme = lexeme;

      if (lexeme) {
        if (lexeme->type == Lexeme::type_separator) {
          if (lexeme->value == ",") {
            cpu.addCall(def_XBASIC_PRINT_TAB);  // call print_tab
          } else if (lexeme->value == ";") {
            continue;
          } else if (lexeme->value == "#") {
            if (context->has_open_grp) continue;

            redirected = true;
            subaction = action->actions[0];
            result_subtype = expression.evalExpression(subaction);
            expression.addCast(result_subtype, Lexeme::subtype_numeric);

            // call io redirect
            if (context->io_redirect_mark)
              fixup.addFix(context->io_redirect_mark->symbol);
            else
              context->io_redirect_mark = fixup.addMark();
            cpu.addCall(0x0000);

            continue;
          } else {
            context->syntaxError("Invalid PRINT parameter separator");
            return;
          }
        } else {
          result_subtype = expression.evalExpression(action);

          if (result_subtype == Lexeme::subtype_string) {
            cpu.addCall(def_XBASIC_PRINT_STR);  // call print_str

          } else if (result_subtype == Lexeme::subtype_numeric) {
            cpu.addCall(def_XBASIC_PRINT_INT);  // call print_int

          } else if (result_subtype == Lexeme::subtype_single_decimal ||
                     result_subtype == Lexeme::subtype_double_decimal) {
            cpu.addCall(def_XBASIC_PRINT_FLOAT);  // call print_float

          } else {
            context->syntaxError("Invalid PRINT parameter");
            return;
          }
        }
      }
    }

  } else {
    cpu.addCall(def_XBASIC_PRINT_CRLF);  // call print_crlf
  }

  if (last_lexeme) {
    if (last_lexeme->type != Lexeme::type_separator ||
        (last_lexeme->value != ";" && last_lexeme->value != ",")) {
      cpu.addCall(def_XBASIC_PRINT_CRLF);  // call print_crlf
    }
  }

  if (redirected) {
    // call io screen
    if (context->io_screen_mark)
      fixup.addFix(context->io_screen_mark->symbol);
    else
      context->io_screen_mark = fixup.addMark();
    cpu.addCall(0x0000);
  }
}

void CompilerStatementEmitter::cmd_input(bool question) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
  Lexeme* lexeme;
  ActionNode *action, *subaction;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;
  bool redirected = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        if (lexeme->type == Lexeme::type_separator) {
          if (lexeme->value == ",") {
            cpu.addCall(def_XBASIC_PRINT_TAB);  // call print_tab
          } else if (lexeme->value == ";") {
            continue;
          } else if (lexeme->value == "#") {
            redirected = true;
            subaction = action->actions[0];
            result_subtype = expression.evalExpression(subaction);
            expression.addCast(result_subtype, Lexeme::subtype_numeric);

            // call io redirect
            if (context->io_redirect_mark)
              fixup.addFix(context->io_redirect_mark->symbol);
            else
              context->io_redirect_mark = fixup.addMark();
            cpu.addCall(0x0000);

            continue;
          } else {
            context->syntaxError("Invalid INPUT parameter separator");
            return;
          }
        } else {
          if (lexeme->type == Lexeme::type_identifier) {
            // call INPUT or LINE INPUT

            if (question) {
              cpu.addCall(def_XBASIC_INPUT_1);
            } else {
              cpu.addCall(def_XBASIC_INPUT_2);
            }

            // do assignment

            expression.addCast(Lexeme::subtype_string, lexeme->subtype);

            if (!context->variableEmitter->addAssignment(action)) return;

          } else {
            result_subtype = expression.evalExpression(action);

            if (result_subtype == Lexeme::subtype_string) {
              cpu.addCall(def_XBASIC_PRINT_STR);  // call print_str

            } else if (result_subtype == Lexeme::subtype_numeric) {
              cpu.addCall(def_XBASIC_PRINT_INT);  // call print_int

            } else if (result_subtype == Lexeme::subtype_single_decimal ||
                       result_subtype == Lexeme::subtype_double_decimal) {
              cpu.addCall(def_XBASIC_PRINT_FLOAT);  // call print_float

            } else {
              context->syntaxError("Invalid INPUT parameter");
              return;
            }
          }
        }
      }
    }

    if (redirected) {
      // call io screen
      if (context->io_screen_mark)
        fixup.addFix(context->io_screen_mark->symbol);
      else
        context->io_screen_mark = fixup.addMark();
      cpu.addCall(0x0000);
    }

  } else {
    context->syntaxError();
  }
}

void CompilerStatementEmitter::cmd_line() {
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
      context->current_action = action;
      return cmd_input(false);
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

void CompilerStatementEmitter::cmd_copy() {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
  ActionNode *action, *sub_action;
  Lexeme* lexeme;
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
            return cmd_copy_screen();
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

void CompilerStatementEmitter::cmd_copy_screen() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  int t = context->current_action->actions.size();

  if (t > 1) {
    context->syntaxError("Invalid COPY SCREEN parameters");
  } else {
    if (t == 0) {
      // xor a
    } else if (t == 1) {
      ActionNode *action = context->current_action->actions[0], *sub_action;
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

void CompilerStatementEmitter::cmd_sound() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        result_subtype = expression.evalExpression(action);

        // cast
        expression.addCast(result_subtype, Lexeme::subtype_numeric);

        if (i == 0) {
          // ld a, l
          cpu.addLdAL();
          // push af
          cpu.addPushAF();
        } else {
          // ld e, l
          cpu.addLdEL();
          // pop af
          cpu.addPopAF();

          // call sound function
          cpu.addCall(def_XBASIC_SOUND);
        }
      }
    }

  } else {
    context->syntaxError("Invalid SOUND parameters");
  }
}

void CompilerStatementEmitter::cmd_bload() {
  auto& cpu = *context->cpu;
  auto& opts = *context->opts;
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int t = context->current_action->actions.size();
  bool isTinySprite;
  int resource_number;
  string filename, fileext;

  if (t == 2) {
    action = context->current_action->actions[1];
    lexeme = action->lexeme;
    if (lexeme) {
      if (lexeme->name == "S") {
        action = context->current_action->actions[0];
        lexeme = action->lexeme;
        if (lexeme) {
          if (lexeme->type == Lexeme::type_literal &&
              lexeme->subtype == Lexeme::subtype_string) {
            // add to resource list

            // verify file type (screen or sprite)

            filename = removeQuotes(lexeme->value);
            fileext = getFileExtension(filename);
            isTinySprite = (strcasecmp((char*)fileext.c_str(), ".SPR") == 0);

            resource_number = context->resourceManager.resources.size();
            lexeme->name = "FILE";
            context->resourceManager.addFile(filename, opts.inputPath);

            // execute a resource screen load

            // ld hl, resource number
            cpu.addLdHL(resource_number);

            // ld (DAC), hl
            cpu.addLdiiHL(def_DAC);

            if (isTinySprite) {
              // call cmd_wrtspr                    ; tiny sprite loader
              cpu.addCall(def_cmd_wrtspr);
            } else {
              // call screen_load
              cpu.addCall(def_cmd_screen_load);
            }

          } else {
            context->syntaxError(
                "BLOAD 1st parameter must be a string constant");
          }

        } else {
          context->syntaxError("BLOAD 1st parameter error");
        }

      } else {
        context->syntaxError("BLOAD valid only to screen");
      }

    } else {
      context->syntaxError("BLOAD 2nd parameter error");
    }

  } else {
    context->syntaxError("Invalid BLOAD parameters count");
  }
}

void CompilerStatementEmitter::cmd_play() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode* action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;
  bool xor_a = true;

  if (t >= 1 && t <= 3) {
    for (i = 0; i < 3; i++) {
      if (i < t) {
        action = context->current_action->actions[i];
        result_subtype = expression.evalExpression(action);
        if (result_subtype != Lexeme::subtype_string) {
          context->syntaxError("Invalid PLAY parameter");
          return;
        }
        // push hl
        cpu.addPushHL();
      } else {
        if (xor_a) {
          // xor a
          cpu.addXorA();
          xor_a = false;
        }
        // push af
        cpu.addPushAF();
      }
    }

    // pop bc
    cpu.addPopBC();
    // pop de
    cpu.addPopDE();
    // pop hl
    cpu.addPopHL();

    cpu.addCall(def_XBASIC_PLAY);

  } else {
    context->syntaxError("Invalid PLAY parameters");
  }
}

void CompilerStatementEmitter::cmd_draw() {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t == 1) {
    i = 0;
    action = context->current_action->actions[i];
    lexeme = action->lexeme;
    if (lexeme) {
      result_subtype = expression.evalExpression(action);

      if (result_subtype == Lexeme::subtype_string) {
        // call draw function
        if (context->draw_mark)
          fixup.addFix(context->draw_mark->symbol);
        else
          context->draw_mark = fixup.addMark();
        cpu.addCall(0x0000);

      } else {
        context->syntaxError("Invalid DRAW parameter");
        return;
      }
    }

  } else {
    context->syntaxError("Invalid DRAW parameters");
  }
}

void CompilerStatementEmitter::cmd_locate() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& optimizer = *context->codeOptimizer;
  ActionNode* action;
  unsigned int t = context->current_action->actions.size();
  int result_subtype;

  if (t != 2) {
    context->syntaxError("LOCATE without enough parameters");
  } else {
    action = context->current_action->actions[0];
    result_subtype = expression.evalExpression(action);

    expression.addCast(result_subtype, Lexeme::subtype_numeric);

    // push hl
    cpu.addPushHL();

    action = context->current_action->actions[1];
    result_subtype = expression.evalExpression(action);

    expression.addCast(result_subtype, Lexeme::subtype_numeric);

    // pop de
    optimizer.addByteOptimized(0xD1);
    // call XBASIC_LOCATE    ; hl = y, de = x
    cpu.addCall(def_XBASIC_LOCATE);
  }
}

void CompilerStatementEmitter::cmd_screen() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode* action;
  Lexeme* lexeme;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t) {
    action = context->current_action->actions[0];
    lexeme = action->lexeme;
    if (lexeme->value == "COPY") {
      context->current_action = action;
      cmd_screen_copy();
      return;
    } else if (lexeme->value == "PASTE") {
      context->current_action = action;
      cmd_screen_paste();
      return;
    } else if (lexeme->value == "SCROLL") {
      context->current_action = action;
      cmd_screen_scroll();
      return;
    } else if (lexeme->value == "LOAD") {
      context->current_action = action;
      cmd_screen_load();
      return;
    } else if (lexeme->value == "ON") {
      context->current_action = action;
      cmd_screen_on();
      return;
    } else if (lexeme->value == "OFF") {
      context->current_action = action;
      cmd_screen_off();
      return;
    }

    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      result_subtype = expression.evalExpression(action);

      if (result_subtype == Lexeme::subtype_null) continue;

      expression.addCast(result_subtype, Lexeme::subtype_numeric);

      if (i != 5) {
        // ld a, l
        cpu.addLdAL();
      }

      switch (i) {
        // display mode
        case 0: {
          // call XBASIC_SCREEN ; xbasic SCREEN change mode (in: a, l = screen
          // mode)
          cpu.addCall(def_XBASIC_SCREEN);
        } break;

        // sprite size
        case 1: {
          // call 0x70bc    ; xbasic SCREEN sprite (in: a = sprite mode)
          cpu.addCall(def_XBASIC_SCREEN_SPRITE);
        } break;

        // key click
        case 2: {
          // ld (CLIKSW), a   ; 0=keyboard click off, 1=keyboard click on
          cpu.addLdiiA(def_CLIKSW);
        } break;

        // baud rate
        case 3: {
          // ; original code: C1F63 on subrom of TurboR and A7A2D on main rom of
          // the rest ld bc,5
          cpu.addLdBC(0x0005);
          // and a
          cpu.addAndA();
          // ld hl,CS1200
          cpu.addLdHL(def_CS1200);
          // jr z,skip
          cpu.addJrZ(0x01);
          //   add hl,bc
          cpu.addAddHLBC();
          // skip:
          // ld de,LOW
          cpu.addLdDE(def_LOW);
          // ldir
          cpu.addLDIR();
        } break;

        // printer type
        case 4: {
          // ld (NTMSXP), a   ; printer type (0=default)
          cpu.addLdiiA(def_NTMSXP);
        } break;

        // interlace mode
        case 5: {
          // ld a, (VERSION)
          cpu.addLdAii(def_VERSION);
          // and a
          cpu.addAndA();
          // jr z, skip1
          cpu.addJrZ(25);

          //   ld a, l
          cpu.addLdAL();

          //   ; original code: J1F45 on subrom of MSX2 and above
          //   and 3     ; must be 0 to 3
          cpu.addAnd(0x03);
          //   add a, a
          cpu.addAddA();
          //   bit 1, a
          cpu.addWord(0xCB, 0x4F);
          //   jr z, skip2
          cpu.addJrZ(0x02);
          //     set 3, a
          cpu.addWord(0xCB, 0xDF);
          //   skip2:
          //   and 0x0C
          cpu.addAnd(0x0C);
          //   ld b, a
          cpu.addLdBA();
          //   ld a, (RG9SAV)
          cpu.addLdAii(def_RG9SAV);
          //   and 0xF3
          cpu.addAnd(0xF3);
          //   or b
          cpu.addOrB();
          //   ld c, 9
          cpu.addLdC(0x09);
          //   ld b, a
          cpu.addLdBA();
          //   call WRTVDP
          cpu.addCall(def_WRTVDP);

          // skip1:
        } break;

        default: {
          context->syntaxError("SCREEN parameters not supported");
          return;
        }
      }
    }

  } else {
    context->syntaxError("SCREEN with empty parameters");
  }
}

void CompilerStatementEmitter::cmd_screen_copy() {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t) {
    if (t > 2) {
      context->syntaxError("SCREEN COPY with excess of parameters");
      return;
    }

    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;

      if (i) {
        // push hl
        cpu.addPushHL();

        result_subtype = expression.evalExpression(action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);

        // ld a, l                 ; copy parameter to A
        cpu.addLdAL();

        // pop hl
        cpu.addPopHL();
      } else {
        if (lexeme->type == Lexeme::type_identifier) {
          // ld hl, variable
          fixup.addFix(lexeme);
          cpu.addLdHL(0x0000);
          result_subtype = Lexeme::subtype_numeric;
        } else {
          result_subtype = expression.evalExpression(action);
        }
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        if (t == 1) {
          // xor a
          cpu.addXorA();
        }
      }
    }

    // call screen_copy
    cpu.addCall(def_cmd_screen_copy);

  } else {
    context->syntaxError("SCREEN COPY with empty parameters");
  }
}

void CompilerStatementEmitter::cmd_screen_paste() {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t) {
    if (t > 1) {
      context->syntaxError("SCREEN PASTE with excess of parameters");
      return;
    }

    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->type == Lexeme::type_identifier) {
        // ld hl, variable
        fixup.addFix(lexeme);
        cpu.addLdHL(0x0000);
        result_subtype = Lexeme::subtype_numeric;
      } else {
        result_subtype = expression.evalExpression(action);
      }
      expression.addCast(result_subtype, Lexeme::subtype_numeric);
    }

    // call screen_paste
    cpu.addCall(def_cmd_screen_paste);

  } else {
    context->syntaxError("SCREEN PASTE with empty parameters");
  }
}

void CompilerStatementEmitter::cmd_screen_scroll() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode* action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t) {
    if (t > 1) {
      context->syntaxError("SCREEN SCROLL with excess of parameters");
      return;
    }

    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      result_subtype = expression.evalExpression(action);
      expression.addCast(result_subtype, Lexeme::subtype_numeric);

      // ld a, l                 ; copy parameter to A
      cpu.addLdAL();

      // ld hl, (HEAPSTR)
      cpu.addLdHLii(def_HEAPSTR);

      // push hl
      cpu.addPushHL();

      // call screen_copy
      cpu.addCall(def_cmd_screen_copy);

      // pop hl
      cpu.addPopHL();

      // call screen_paste
      cpu.addCall(def_cmd_screen_paste);
    }

  } else {
    context->syntaxError("SCREEN SCROLL with empty parameters");
  }
}

void CompilerStatementEmitter::cmd_screen_load() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode* action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      result_subtype = expression.evalExpression(action);
      expression.addCast(result_subtype, Lexeme::subtype_numeric);

      // ld (DAC), hl
      cpu.addLdiiHL(def_DAC);

      // call screen_load
      cpu.addCall(def_cmd_screen_load);
    }

  } else {
    context->syntaxError("SCREEN LOAD with empty parameters");
  }
}

void CompilerStatementEmitter::cmd_screen_on() {
  auto& cpu = *context->cpu;
  // call cmd_enascr
  cpu.addCall(def_cmd_enascr);
}

void CompilerStatementEmitter::cmd_screen_off() {
  auto& cpu = *context->cpu;
  // call cmd_disscr
  cpu.addCall(def_cmd_disscr);
}

void CompilerStatementEmitter::cmd_width() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode* action;
  unsigned int t = context->current_action->actions.size();
  int result_subtype;

  if (t == 1) {
    action = context->current_action->actions[0];
    result_subtype = expression.evalExpression(action);

    expression.addCast(result_subtype, Lexeme::subtype_numeric);

    // call XBASIC WIDTH   ; xbasic WIDTH (in: l = size)
    cpu.addCall(def_XBASIC_WIDTH);

  } else {
    context->syntaxError("WIDTH syntax error");
  }
}

void CompilerStatementEmitter::cmd_color() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode *action, *subaction;
  Lexeme* lexeme;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t) {
    action = context->current_action->actions[0];
    lexeme = action->lexeme;

    if (lexeme->type == Lexeme::type_keyword) {
      if (lexeme->value == "NEW") {
        // ld a, (VERSION)
        cpu.addLdAii(def_VERSION);
        // and a
        cpu.addAndA();
        // jr z, skip
        cpu.addJrZ(8);
        //   ld ix, 0x0141
        cpu.addLdIX(0x0141);
        //   call EXTROM
        cpu.addCall(def_EXTROM);
        //   ei
        cpu.addEI();
        // skip:

      } else if (lexeme->value == "RESTORE") {
        // ld a, (VERSION)
        cpu.addLdAii(def_VERSION);
        // and a
        cpu.addAndA();
        // jr z, skip
        cpu.addJrZ(8);
        //   ld ix, 0x0145
        cpu.addLdIX(def_RSTPLT);
        //   call EXTROM
        cpu.addCall(def_EXTROM);
        //   ei
        cpu.addEI();
        // skip:

      } else if (lexeme->value == "SPRITE") {
        t = action->actions.size();

        if (t != 2) {
          context->syntaxError("Invalid COLOR SPRITE parameters count");
          return;
        }

        for (i = 0; i < t; i++) {
          subaction = action->actions[i];
          result_subtype = expression.evalExpression(subaction);

          if (result_subtype != Lexeme::subtype_null) {
            expression.addCast(result_subtype, Lexeme::subtype_numeric);

            // ld a, l
            cpu.addLdAL();
          }

          if (i < 1) {
            // push af
            cpu.addPushAF();
          }
        }

        // pop bc
        cpu.addPopBC();
        // call COLOR_SPRITE   ; in: b, a
        cpu.addCall(def_XBASIC_COLOR_SPRITE);

      } else if (lexeme->value == "SPRITE$") {
        t = action->actions.size();

        if (t != 2) {
          context->syntaxError("Invalid COLOR SPRITE$ parameters count");
          return;
        }

        for (i = 0; i < t; i++) {
          subaction = action->actions[i];
          result_subtype = expression.evalExpression(subaction);

          if (i < 1) {
            expression.addCast(result_subtype, Lexeme::subtype_numeric);

            // ld a, l
            cpu.addLdAL();
            // push af
            cpu.addPushAF();
          }
        }

        // pop bc
        cpu.addPopBC();
        // call COLOR_SPRSTR   ; in: b, hl
        cpu.addCall(def_XBASIC_COLOR_SPRSTR);

      } else if (lexeme->value == "RGB") {
        t = action->actions.size();

        if (t < 2 || t > 4) {
          context->syntaxError("Invalid COLOR RGB parameters count");
          return;
        }

        for (i = 0; i < t; i++) {
          subaction = action->actions[i];
          result_subtype = expression.evalExpression(subaction);

          if (result_subtype != Lexeme::subtype_null) {
            expression.addCast(result_subtype, Lexeme::subtype_numeric);

            // ld a, l
            cpu.addLdAL();
          }

          if (i < 3) {
            // push af
            cpu.addPushAF();
          }
        }

        if (t < 4) {
          // xor a
          cpu.addXorA();
        }
        // ld b, a
        cpu.addLdBA();

        if (t < 3) {
          // ld h, a
          cpu.addLdHA();
        } else {
          // pop hl
          cpu.addPopHL();
        }

        // pop de
        cpu.addPopDE();
        // pop af
        cpu.addPopAF();

        // call COLOR_RGB   ; in: a, d, h, b
        cpu.addCall(def_XBASIC_COLOR_RGB);

      } else {
        context->syntaxError("Invalid COLOR parameters");
      }

    } else {
      for (i = 0; i < t; i++) {
        action = context->current_action->actions[i];
        result_subtype = expression.evalExpression(action);

        if (result_subtype == Lexeme::subtype_null) continue;

        expression.addCast(result_subtype, Lexeme::subtype_numeric);

        // ld a, l
        cpu.addLdAL();

        switch (i) {
          case 0: {
            // ld (0xF3E9), a   ; FORCLR
            cpu.addLdiiA(0xF3E9);
            // ld (ATRBYT), a   ; ATRBYT
            cpu.addLdiiA(def_ATRBYT);
          } break;

          case 1: {
            // ld (0xF3EA), a   ; BAKCLR
            cpu.addLdiiA(0xF3EA);
          } break;

          case 2: {
            // ld (0xF3EB), a   ; BDRCLR
            cpu.addLdiiA(0xF3EB);
          } break;

          default: {
            context->syntaxError("COLOR parameters not supported");
            return;
          }
        }
      }

      // ld a, (SCRMOD)
      cpu.addLdAii(def_SCRMOD);

      // call 0x0062   ; CHGCLR
      cpu.addCall(0x0062);
    }

  } else {
    context->syntaxError("COLOR with empty parameters");
  }
}

void CompilerStatementEmitter::cmd_pset(bool forecolor) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode *action, *sub_action;
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

void CompilerStatementEmitter::cmd_paint() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode *action, *sub_action;
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

void CompilerStatementEmitter::cmd_circle() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& optimizer = *context->codeOptimizer;
  ActionNode *action, *sub_action;
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

void CompilerStatementEmitter::cmd_put() {
  ActionNode* action;
  Lexeme* lexeme;
  unsigned int t = context->current_action->actions.size();

  if (t) {
    action = context->current_action->actions[0];
    lexeme = action->lexeme;

    if (lexeme->type == Lexeme::type_keyword && lexeme->value == "SPRITE") {
      context->current_action = action;
      cmd_put_sprite();
    } else if (lexeme->type == Lexeme::type_keyword &&
               lexeme->value == "TILE") {
      context->current_action = action;
      cmd_put_tile();
    } else {
      context->syntaxError("Invalid PUT statement");
    }

  } else {
    context->syntaxError("Empty PUT statement");
  }
}

void CompilerStatementEmitter::cmd_put_sprite() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode *action, *sub_action;
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

void CompilerStatementEmitter::cmd_put_tile() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode *action, *sub_action;
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

void CompilerStatementEmitter::cmd_set() {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  ActionNode* action;
  Lexeme* next_lexeme;
  unsigned int t = context->current_action->actions.size();
  FixNode* mark;

  if (t == 1) {
    action = context->current_action->actions[0];
    next_lexeme = action->lexeme;

    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "TILE") {
        cmd_set_tile();
        return;
      } else if (next_lexeme->value == "SPRITE") {
        cmd_set_sprite();
        return;
      } else if (next_lexeme->value == "FONT") {
        cmd_set_font();
        return;
      }
    }

    // ld a, (BIOS VERSION)
    cpu.addLdAii(def_VERSION);
    // and a
    cpu.addAndA();
    // jp z, $                ; skip if MSX1
    mark = fixup.addMark();
    cpu.addJpZ(0x0000);

    if (next_lexeme->type == Lexeme::type_keyword &&
        next_lexeme->value == "ADJUST") {
      cmd_set_adjust();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "BEEP") {
      cmd_set_beep();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "DATE") {
      cmd_set_date();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "PAGE") {
      cmd_set_page();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "PASSWORD") {
      context->syntaxError(
          "SET PASSWORD will not be supported for end-user security");
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "PROMPT") {
      cmd_set_prompt();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "SCREEN") {
      cmd_set_screen();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "SCROLL") {
      cmd_set_scroll();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "TIME") {
      cmd_set_time();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "TITLE") {
      cmd_set_title();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "VIDEO") {
      cmd_set_video();
    } else {
      context->syntaxError("Invalid SET statement");
    }

    mark->symbol->address = cpu.context->code_pointer;

  } else {
    context->syntaxError("Wrong SET parameters count");
  }
}

void CompilerStatementEmitter::cmd_set_video() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& codeHelper = *context->codeHelper;
  ActionNode *action = context->current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();
  int result_subtype;

  if (t) {
    if (t > 7) {
      context->syntaxError("Invalid SET VIDEO parameters");
      return;
    }

    codeHelper.beginBasicSetStmt("VIDEO");

    for (i = 0; i < t; i++) {
      if (i) {
        // comma
        codeHelper.addBasicChar(',');
      }

      // push hl
      cpu.addPushHL();

      sub_action = action->actions[i];
      result_subtype = expression.evalExpression(sub_action);

      if (result_subtype == Lexeme::subtype_null) {
        cpu.context->code_pointer--;
        cpu.context->code_size--;
        continue;
      }

      expression.addCast(result_subtype, Lexeme::subtype_numeric);

      //   ld a, l
      cpu.addLdAL();
      //   and 3
      cpu.addAnd(0x03);
      //   inc a
      cpu.addIncA();
      //   or 0x10
      cpu.addOr(0x10);

      // pop hl
      cpu.addPopHL();

      // ld (hl), a
      cpu.addLdiHLA();
      // inc hl
      cpu.addIncHL();
    }

    codeHelper.endBasicSetStmt();

  } else {
    context->syntaxError("SET VIDEO with empty parameters");
  }
}

void CompilerStatementEmitter::cmd_set_adjust() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& codeHelper = *context->codeHelper;
  ActionNode *action = context->current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();
  int result_subtype;

  if (t == 2) {
    // get parameters

    for (i = 0; i < t; i++) {
      if (i) {
        // push hl
        cpu.addPushHL();
      }
      // ld hl, parameter value
      sub_action = action->actions[i];
      result_subtype = expression.evalExpression(sub_action);
      expression.addCast(result_subtype, Lexeme::subtype_numeric);
    }

    // ex de,hl   ; DE = second parameter
    cpu.addExDEHL();
    // pop bc     ; BC = first parameter
    cpu.addPopBC();

    // build command string

    codeHelper.beginBasicSetStmt("ADJUST");

    // (
    codeHelper.addBasicChar('(');

    // integer prefix
    codeHelper.addBasicChar(0x1C);
    // ld (hl), c      ; first parameter
    cpu.addLdiHLC();
    // inc hl
    cpu.addIncHL();
    // ld (hl), b
    cpu.addLdiHLB();
    // inc hl
    cpu.addIncHL();

    // comma
    codeHelper.addBasicChar(',');

    // integer prefix
    codeHelper.addBasicChar(0x1C);
    // ld (hl), e      ; second parameter
    cpu.addLdiHLE();
    // inc hl
    cpu.addIncHL();
    // ld (hl), d
    cpu.addLdiHLD();
    // inc hl
    cpu.addIncHL();

    // )
    codeHelper.addBasicChar(')');

    codeHelper.endBasicSetStmt();

  } else {
    context->syntaxError("Wrong parameters count on SET ADJUST statement");
  }
}

void CompilerStatementEmitter::cmd_set_screen() {
  auto& codeHelper = *context->codeHelper;
  ActionNode* action = context->current_action->actions[0];
  unsigned int t = action->actions.size();

  if (t == 0) {
    // build command string

    codeHelper.beginBasicSetStmt("");
    codeHelper.addBasicChar(0xC5);  // token for SCREEN

    codeHelper.endBasicSetStmt();

  } else {
    context->syntaxError("Wrong parameters count on SET SCREEN statement");
  }
}

void CompilerStatementEmitter::cmd_set_beep() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& codeHelper = *context->codeHelper;
  ActionNode *action = context->current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();
  int result_subtype;

  if (t == 2) {
    // get parameters

    for (i = 0; i < t; i++) {
      if (i) {
        // push hl
        cpu.addPushHL();
      }
      // ld hl, parameter value
      sub_action = action->actions[i];
      result_subtype = expression.evalExpression(sub_action);
      expression.addCast(result_subtype, Lexeme::subtype_numeric);
    }

    // ex de,hl   ; DE = second parameter
    cpu.addExDEHL();
    // pop bc     ; BC = first parameter
    cpu.addPopBC();

    // build command string

    codeHelper.beginBasicSetStmt("");
    codeHelper.addBasicChar(0xC0);  // token for BEEP

    // short integer prefix
    codeHelper.addBasicChar(0x0F);
    // ld (hl), c      ; first parameter
    cpu.addLdiHLC();
    // inc hl
    cpu.addIncHL();

    // comma
    codeHelper.addBasicChar(',');

    // short integer prefix
    codeHelper.addBasicChar(0x0F);
    // ld (hl), e      ; second parameter
    cpu.addLdiHLE();
    // inc hl
    cpu.addIncHL();

    codeHelper.endBasicSetStmt();

  } else {
    context->syntaxError("Wrong parameters count on SET BEEP statement");
  }
}

void CompilerStatementEmitter::cmd_set_title() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& codeHelper = *context->codeHelper;
  ActionNode *action = context->current_action->actions[0], *sub_action;
  unsigned int t = action->actions.size();
  int result_subtype;

  if (t == 2) {
    // get parameters

    // ld hl, parameter value
    sub_action = action->actions[1];
    result_subtype = expression.evalExpression(sub_action);
    expression.addCast(result_subtype, Lexeme::subtype_numeric);

    // push hl
    cpu.addPushHL();

    // ld hl, parameter value
    sub_action = action->actions[0];
    result_subtype = expression.evalExpression(sub_action);
    expression.addCast(result_subtype, Lexeme::subtype_string);

    // ex de,hl   ; DE = first parameter
    cpu.addExDEHL();
    // pop bc     ; BC = second parameter
    cpu.addPopBC();

    // build command string

    codeHelper.beginBasicSetStmt("TITLE");

    // double quote
    codeHelper.addBasicChar('"');
    // push bc
    cpu.addPushBC();
    // ex de,hl
    cpu.addExDEHL();
    //   ld c, (hl)  ; string size
    cpu.addLdCiHL();
    //   ld b, 0
    cpu.addLdB(0x00);
    //   inc hl
    cpu.addIncHL();
    //   ldir
    cpu.addLDIR();
    // ex de,hl
    cpu.addExDEHL();
    // pop bc
    cpu.addPopBC();
    // double quote
    codeHelper.addBasicChar('"');

    // comma
    codeHelper.addBasicChar(',');

    // short integer prefix
    codeHelper.addBasicChar(0x0F);
    // ld (hl), c      ; second parameter
    cpu.addLdiHLC();
    // inc hl
    cpu.addIncHL();

    codeHelper.endBasicSetStmt();

  } else {
    context->syntaxError("Wrong parameters count on SET TITLE statement");
  }
}

void CompilerStatementEmitter::cmd_set_prompt() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& codeHelper = *context->codeHelper;
  ActionNode *action = context->current_action->actions[0], *sub_action;
  unsigned int t = action->actions.size();
  int result_subtype;

  if (t == 1) {
    // get parameters

    // ld hl, parameter value
    sub_action = action->actions[0];
    result_subtype = expression.evalExpression(sub_action);
    expression.addCast(result_subtype, Lexeme::subtype_string);

    // ex de,hl   ; DE = first parameter
    cpu.addExDEHL();

    // build command string

    codeHelper.beginBasicSetStmt("PROMPT");

    // double quote
    codeHelper.addBasicChar('"');
    // ex de,hl
    cpu.addExDEHL();
    //   ld c, (hl)  ; string size
    cpu.addLdCiHL();
    //   ld b, 0
    cpu.addLdB(0x00);
    //   inc hl
    cpu.addIncHL();
    //   ldir
    cpu.addLDIR();
    // ex de,hl
    cpu.addExDEHL();
    // double quote
    codeHelper.addBasicChar('"');

    codeHelper.endBasicSetStmt();

  } else {
    context->syntaxError("Wrong parameters count on SET PROMPT statement");
  }
}

void CompilerStatementEmitter::cmd_set_page() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode *action = context->current_action->actions[0], *sub_action;
  Lexeme* lexeme;
  unsigned int t = action->actions.size();
  int result_subtype;

  if (t >= 1 && t <= 2) {
    // ld hl, parameter value
    sub_action = action->actions[0];
    lexeme = sub_action->lexeme;

    if (!(lexeme->type == Lexeme::type_literal &&
          lexeme->subtype == Lexeme::subtype_null)) {
      result_subtype = expression.evalExpression(sub_action);
      expression.addCast(result_subtype, Lexeme::subtype_numeric);

      // ld a,l
      cpu.addLdAL();

      // call SET_PAGE       ; in: a = display page
      cpu.addCall(def_XBASIC_SET_PAGE);
    }

    if (t == 2) {
      sub_action = action->actions[1];
      result_subtype = expression.evalExpression(sub_action);
      expression.addCast(result_subtype, Lexeme::subtype_numeric);

      //   ld a,l
      cpu.addLdAL();
      //   ld (ACPAGE), a    ; in: a = active page (write and read)
      cpu.addLdiiA(def_ACPAGE);
    }

  } else {
    context->syntaxError("Wrong parameters count on SET PAGE statement");
  }
}

void CompilerStatementEmitter::cmd_set_scroll() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode *action = context->current_action->actions[0], *sub_action;
  Lexeme* lexeme;
  unsigned int i, t = action->actions.size();
  int result_subtype;

  if (t > 0 && t <= 4) {
    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      lexeme = sub_action->lexeme;

      if (lexeme->type == Lexeme::type_literal &&
          lexeme->subtype == Lexeme::subtype_null) {
        // ld hl, 0xffff
        cpu.addLdHL(0xFFFF);
      } else {
        // ld hl, parameter value
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
      }

      if (i > 1) {
        // ld h, l
        cpu.addLdHL();
      }

      // push hl
      cpu.addPushHL();
    }

    if (t == 4) {
      // pop af
      cpu.addPopAF();
    } else {
      // ld a, 0xff
      cpu.addLdA(0xFF);
    }

    if (t >= 3) {
      // pop bc
      cpu.addPopBC();
    } else {
      // ld b, 0xff
      cpu.addLdB(0xFF);
    }

    if (t >= 2) {
      // pop hl
      cpu.addPopHL();
    } else {
      // ld hl, 0xffff
      cpu.addLdHL(0xFFFF);
    }

    // pop de
    cpu.addPopDE();

    // call SET_SCROLL      ; in: de=x, hl=y, b=mask mode, a=page mode
    cpu.addCall(def_XBASIC_SET_SCROLL);

  } else {
    context->syntaxError("Wrong parameters count on SET SCROLL statement");
  }
}

void CompilerStatementEmitter::cmd_set_tile() {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
  ActionNode *action = context->current_action->actions[0], *sub_action,
             *sub_sub_action;
  Lexeme *lexeme, *sub_lexeme;
  unsigned int i, t, tt;
  int result_subtype;

  t = action->actions.size();
  if (t) {
    action = action->actions[0];
    lexeme = action->lexeme;
    t = action->actions.size();

    if (lexeme->value == "ON") {
      CompilerClsStatementStrategy clsStmt;
      // ld a, 2                   ; tiled mode
      cpu.addLdA(2);
      // ld (SOMODE), a
      cpu.addLdiiA(def_SOMODE);
      // clear screen, set font to default and put cursor on home
      clsStmt.execute(context);
      // ld hl, 0
      cpu.addLdHL(0x0000);
      // ld (DAC), hl
      cpu.addLdiiHL(def_DAC);
      // ld d, h
      cpu.addLdDH();
      // ld e, l
      cpu.addLdEL();
      // call XBASIC_LOCATE    ; hl = y, de = x
      cpu.addCall(def_XBASIC_LOCATE);
      // ld a, 0xff                ; it means all screen banks
      cpu.addLdA(0xFF);
      // ld (ARG), a
      cpu.addLdiiA(def_ARG);
      // call cmd_setfnt
      cpu.addCall(def_cmd_setfnt);

    } else if (lexeme->value == "OFF") {
      // ld a, 1      ; graphical mode
      cpu.addLdA(1);
      // ld (SOMODE), a
      cpu.addLdiiA(def_SOMODE);

    } else if (lexeme->value == "FLIP") {
      if (t == 2) {
        // tile number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; parameter
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        // push hl
        cpu.addPushHL();

        // direction
        sub_action = action->actions[1];
        // ld hl, parameter value    ; parameter
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        // pop de
        cpu.addPopDE();

        cpu.addCall(def_set_tile_flip);

      } else {
        context->syntaxError(
            "Wrong parameters count on SET TILE FLIP statement");
      }

    } else if (lexeme->value == "ROTATE") {
      if (t == 2) {
        // tile number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; parameter
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        // push hl
        cpu.addPushHL();

        // direction
        sub_action = action->actions[1];
        // ld hl, parameter value    ; parameter
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        // pop de
        cpu.addPopDE();

        cpu.addCall(def_set_tile_rotate);

      } else {
        context->syntaxError(
            "Wrong parameters count on SET TILE ROTATE statement");
      }

    } else if (lexeme->value == "PATTERN") {
      if (t >= 2) {
        sub_action = action->actions[1];
        lexeme = sub_action->lexeme;
        if (t == 2 && lexeme->value != "ARRAY") {
          // tile number
          sub_action = action->actions[0];
          // ld hl, parameter value    ; tile number
          result_subtype = expression.evalExpression(sub_action);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);
          cpu.addLdAL();
          cpu.addPushAF();

          // tile buffer pointer (8 bytes)
          sub_action = action->actions[1];
          sub_lexeme = sub_action->lexeme;
          if (sub_lexeme->type == Lexeme::type_identifier) {
            // ld hl, variable
            fixup.addFix(sub_lexeme);
            cpu.addLdHL(0x0000);
            result_subtype = Lexeme::subtype_numeric;
          } else {
            result_subtype = expression.evalExpression(sub_action);
          }
          expression.addCast(result_subtype, Lexeme::subtype_numeric);
          cpu.addPopAF();

          // call set_tile_pattern
          //   a = tile number
          //   hl = pointer to an 8 bytes buffer
          cpu.addCall(def_set_tile_pattern);
        } else if (t <= 3) {
          // tile number
          sub_action = action->actions[0];
          // ld hl, parameter value    ; tile number
          result_subtype = expression.evalExpression(sub_action);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);
          // ld (ARG), hl
          cpu.addLdiiHL(def_ARG);

          // bank number
          if (t == 3) {
            sub_action = action->actions[2];
            // ld hl, parameter value    ; tile number
            result_subtype = expression.evalExpression(sub_action);
            expression.addCast(result_subtype, Lexeme::subtype_numeric);
            // ld h, l
            cpu.addLdHL();
          } else {
            // ld h, 0x03
            cpu.addLdH(0x03);
          }
          // ld (ARG2), hl
          cpu.addLdiiHL(def_ARG2);

          // pattern data
          sub_action = action->actions[1];
          lexeme = sub_action->lexeme;

          if (lexeme->value == "ARRAY") {
            tt = sub_action->actions.size();

            for (i = 0; i < tt; i++) {
              sub_sub_action = sub_action->actions[i];
              lexeme = sub_sub_action->lexeme;

              if (lexeme->type == Lexeme::type_literal &&
                  lexeme->subtype == Lexeme::subtype_null) {
                continue;

              } else {
                // ld hl, parameter value    ; pattern data parameter
                result_subtype = expression.evalExpression(sub_sub_action);
                expression.addCast(result_subtype, Lexeme::subtype_numeric);

                // ld bc, (ARG2)
                cpu.addLdBCii(def_ARG2);
                // ld c, l
                cpu.addLdCL();
                // ld de, *i*
                cpu.addLdDE(i);
                // ld hl, (ARG)
                cpu.addLdHLii(def_ARG);

                // call set_tile_pattern ; hl = tile number, de = line number, b
                // = bank number (3=all), c = pattern data
                cpu.addCall(def_set_tile_pattern);
              }
            }

          } else {
            context->syntaxError(
                "Wrong pattern parameter on SET TILE PATTERN statement");
          }
        } else {
          context->syntaxError(
              "Wrong parameters count on SET TILE PATTERN statement");
        }
      } else {
        context->syntaxError(
            "Wrong parameters count on SET TILE PATTERN statement");
      }

    } else if (lexeme->value == "COLOR") {
      if (t == 2) {
        // tile number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; tile number
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        cpu.addLdAL();
        cpu.addPushAF();

        // tile buffer pointer (8 bytes)
        sub_action = action->actions[1];
        sub_lexeme = sub_action->lexeme;
        if (sub_lexeme->type == Lexeme::type_identifier) {
          // ld hl, variable
          fixup.addFix(sub_lexeme);
          cpu.addLdHL(0x0000);
          result_subtype = Lexeme::subtype_numeric;
        } else {
          result_subtype = expression.evalExpression(sub_action);
        }
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        cpu.addPopAF();

        // call set_tile_color
        //   a = tile number
        //   hl = pointer to an 8 bytes buffer
        cpu.addCall(def_set_tile_color);
      } else if (t >= 2 && t <= 4) {
        // tile number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; tile number
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        // ld (ARG), hl
        cpu.addLdiiHL(def_ARG);

        // bank number
        if (t == 4) {
          sub_action = action->actions[3];
          // ld hl, parameter value    ; tile number
          result_subtype = expression.evalExpression(sub_action);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);
          // ld h, l
          cpu.addLdHL();
        } else {
          // ld h, 0x03
          cpu.addLdH(0x03);
        }
        // ld (ARG2), hl
        cpu.addLdiiHL(def_ARG2);

        // color data
        sub_action = action->actions[1];
        lexeme = sub_action->lexeme;

        if (lexeme->value == "ARRAY") {
          tt = sub_action->actions.size();

          for (i = 0; i < tt; i++) {
            sub_sub_action = sub_action->actions[i];
            lexeme = sub_sub_action->lexeme;

            if (lexeme->type == Lexeme::type_literal &&
                lexeme->subtype == Lexeme::subtype_null) {
              continue;

            } else {
              // ld hl, parameter value    ; color FC data parameter
              result_subtype = expression.evalExpression(sub_sub_action);
              expression.addCast(result_subtype, Lexeme::subtype_numeric);
              // ld a, l
              cpu.addLdAL();
              // rla
              cpu.addRLA();
              // rla
              cpu.addRLA();
              // rla
              cpu.addRLA();
              // rla
              cpu.addRLA();
              // and 0xF0
              cpu.addAnd(0xF0);

              if (t >= 3) {
                // color data
                sub_sub_action = action->actions[2];
                lexeme = sub_sub_action->lexeme;
                if (lexeme->value != "ARRAY") {
                  context->syntaxError(
                      "Syntax not supported on SET TILE COLOR statement");
                  return;
                }
                if (i < sub_sub_action->actions.size()) {
                  sub_sub_action = sub_sub_action->actions[i];
                  lexeme = sub_sub_action->lexeme;
                  if (!(lexeme->type == Lexeme::type_literal &&
                        lexeme->subtype == Lexeme::subtype_null)) {
                    // push af
                    cpu.addPushAF();
                    // ld hl, parameter value    ; color BC data parameter
                    result_subtype = expression.evalExpression(sub_sub_action);
                    expression.addCast(result_subtype, Lexeme::subtype_numeric);
                    // pop af
                    cpu.addPopAF();
                    // or l
                    cpu.addOrL();
                  }
                }
              }

              // ld bc, (ARG2)
              cpu.addLdBCii(def_ARG2);
              // ld c, a
              cpu.addLdCA();
              // ld de, *i*
              cpu.addLdDE(i);
              // ld hl, (ARG)        ; tile number
              cpu.addLdHLii(def_ARG);

              // call set_tile_color ; hl = tile number, de = line number
              // (15=all), b = bank number (3=all), c = color data (FC,BC)
              cpu.addCall(def_set_tile_color);
            }
          }

        } else {
          // ld hl, parameter value    ; color FC data parameter
          result_subtype = expression.evalExpression(sub_action);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);
          // ld a, l
          cpu.addLdAL();
          // rla
          cpu.addRLA();
          // rla
          cpu.addRLA();
          // rla
          cpu.addRLA();
          // rla
          cpu.addRLA();
          // and 0xF0
          cpu.addAnd(0xF0);

          if (t >= 3) {
            // color data
            sub_sub_action = action->actions[2];
            lexeme = sub_sub_action->lexeme;
            if (lexeme->value == "ARRAY") {
              context->syntaxError(
                  "Syntax not supported on SET TILE COLOR statement");
              return;
            }
            // push af
            cpu.addPushAF();
            // ld hl, parameter value    ; color BC data parameter
            result_subtype = expression.evalExpression(sub_sub_action);
            expression.addCast(result_subtype, Lexeme::subtype_numeric);
            // pop af
            cpu.addPopAF();
            // or l
            cpu.addOrL();
          }

          // ld bc, (ARG2)
          cpu.addLdBCii(def_ARG2);
          // ld c, a
          cpu.addLdCA();
          // ld de, 0x000F         ; all lines
          cpu.addLdDE(0x000F);
          // ld hl, (ARG)        ; tile number
          cpu.addLdHLii(def_ARG);

          // call set_tile_color ; hl = tile number, de = line number
          // (15=all), b = bank number (3=all), c = color data (FC,BC)
          cpu.addCall(def_set_tile_color);
        }

      } else {
        context->syntaxError(
            "Wrong parameters count on SET TILE COLOR statement");
      }

    } else {
      context->syntaxError("Invalid syntax on SET TILE statement");
    }

  } else {
    context->syntaxError("Missing parameters on SET TILE statement");
  }
}

void CompilerStatementEmitter::cmd_set_font() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode *action = context->current_action->actions[0], *sub_action1,
             *sub_action2;
  unsigned int t;
  int result_subtype;

  t = action->actions.size();
  if (t) {
    context->font = true;

    if (t == 1) {
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

    } else if (t == 2) {
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
      context->syntaxError("Wrong number of parameters on SET FONT");
    }

  } else {
    context->syntaxError("SET FONT syntax error");
  }
}

void CompilerStatementEmitter::cmd_set_sprite() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& fixup = *context->fixupResolver;
  ActionNode *action = context->current_action->actions[0], *sub_action;
  Lexeme *lexeme, *sub_lexeme;
  unsigned int t;
  int result_subtype;

  t = action->actions.size();
  if (t) {
    action = action->actions[0];
    lexeme = action->lexeme;
    t = action->actions.size();

    if (lexeme->value == "FLIP") {
      if (t == 2) {
        // sprite number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; parameter
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        // push hl
        cpu.addPushHL();

        // direction
        sub_action = action->actions[1];
        // ld hl, parameter value    ; parameter
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        // pop de
        cpu.addPopDE();

        cpu.addCall(def_set_sprite_flip);

      } else {
        context->syntaxError(
            "Wrong parameters count on SET SPRITE FLIP statement");
      }

    } else if (lexeme->value == "ROTATE") {
      if (t == 2) {
        // sprite number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; parameter
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        // push hl
        cpu.addPushHL();

        // direction
        sub_action = action->actions[1];

        // ld hl, parameter value    ; parameter
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        // pop de
        cpu.addPopDE();

        cpu.addCall(def_set_sprite_rotate);

      } else {
        context->syntaxError(
            "Wrong parameters count on SET SPRITE ROTATE statement");
      }

    } else if (lexeme->value == "PATTERN") {
      if (t == 2) {
        // sprite number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; sprite number
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        cpu.addLdAL();
        cpu.addPushAF();

        // sprite buffer pointer (32 bytes)
        sub_action = action->actions[1];
        sub_lexeme = sub_action->lexeme;
        if (sub_lexeme->type == Lexeme::type_identifier) {
          // ld hl, variable
          fixup.addFix(sub_lexeme);
          cpu.addLdHL(0x0000);
          result_subtype = Lexeme::subtype_numeric;
        } else {
          result_subtype = expression.evalExpression(sub_action);
        }
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        cpu.addPopAF();

        // call set_sprite_pattern
        //   a = sprite number
        //   hl = pointer to a 32 bytes buffer
        cpu.addCall(def_set_sprite_pattern);

      } else {
        context->syntaxError(
            "Wrong parameters count on SET SPRITE PATTERN statement");
      }

    } else if (lexeme->value == "COLOR") {
      if (t == 2) {
        // sprite number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; sprite number
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        cpu.addLdAL();
        cpu.addPushAF();

        // sprite buffer pointer (16 bytes)
        sub_action = action->actions[1];
        sub_lexeme = sub_action->lexeme;
        if (sub_lexeme->type == Lexeme::type_identifier) {
          // ld hl, variable
          fixup.addFix(sub_lexeme);
          cpu.addLdHL(0x0000);
          result_subtype = Lexeme::subtype_numeric;
        } else {
          result_subtype = expression.evalExpression(sub_action);
        }
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        cpu.addPopAF();

        // call set_sprite_color
        //   a = sprite number
        //   hl = pointer to a 16 bytes buffer
        cpu.addCall(def_set_sprite_color);

      } else {
        context->syntaxError(
            "Wrong parameters count on SET SPRITE COLOR statement");
      }

    } else {
      context->syntaxError("Invalid syntax on SET SPRITE statement");
    }
  } else {
    context->syntaxError("Missing parameters on SET SPRITE statement");
  }
}

void CompilerStatementEmitter::cmd_set_date() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  Lexeme* lexeme;
  ActionNode *action = context->current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();
  int result_subtype;

  if (t == 3) {
    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      lexeme = sub_action->lexeme;
      if (lexeme) {
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);

        switch (i) {
          case 0: {
            cpu.addPushHL();
          } break;
          case 1: {
            cpu.addLdDL();
            cpu.addPushDE();
          } break;
          case 2: {
            cpu.addPopDE();
            cpu.addLdEL();
            cpu.addPopHL();
          } break;
        }
      }
    }

    cpu.addCall(def_set_date);

  } else {
    context->syntaxError(
        "Wrong SET DATE parameters count.\nTry: SET DATE iYear, iMonth, "
        "iDay");
  }
}

void CompilerStatementEmitter::cmd_set_time() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  Lexeme* lexeme;
  ActionNode *action = context->current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();
  int result_subtype;

  if (t == 3) {
    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      lexeme = sub_action->lexeme;
      if (lexeme) {
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);

        switch (i) {
          case 0: {
            cpu.addLdHL();
            cpu.addPushHL();
          } break;
          case 1: {
            cpu.addPopDE();
            cpu.addLdEL();
            cpu.addPushDE();
          } break;
          case 2: {
            cpu.addLdAL();
            cpu.addPopHL();
          } break;
        }
      }
    }

    cpu.addCall(def_set_time);

  } else {
    context->syntaxError(
        "Wrong SET TIME parameters count.\nTry: SET TIME iHour, iMinute, "
        "iSecond");
  }
}

void CompilerStatementEmitter::cmd_get() {
  ActionNode* action;
  Lexeme* next_lexeme;
  unsigned int t = context->current_action->actions.size();

  if (t == 1) {
    action = context->current_action->actions[0];
    next_lexeme = action->lexeme;

    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "DATE") {
        cmd_get_date();
      } else if (next_lexeme->value == "TIME") {
        cmd_get_time();
      } else if (next_lexeme->value == "TILE") {
        cmd_get_tile();
      } else if (next_lexeme->value == "SPRITE") {
        cmd_get_sprite();
      } else {
        context->syntaxError("Invalid GET statement");
      }
    } else {
      context->syntaxError("Invalid GET statement");
    }

  } else {
    context->syntaxError("Wrong GET parameters count");
  }
}

void CompilerStatementEmitter::cmd_get_date() {
  auto& cpu = *context->cpu;
  Lexeme* lexeme;
  ActionNode *action = context->current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();

  if (t >= 3 && t <= 5) {
    cpu.addCall(def_get_date);
    cpu.addLdB(0);
    if (t > 4) {
      cpu.addExAF();
      cpu.addLdAii(0x002B);
      cpu.addLdCA();
      cpu.addPushBC();  // date format
      cpu.addExAF();
    }
    if (t > 3) {
      cpu.addLdCA();
      cpu.addPushBC();  // week
    }
    cpu.addLdCE();
    cpu.addPushBC();  // day
    cpu.addLdCD();
    cpu.addPushBC();  // month
    cpu.addPushHL();  // year

    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      lexeme = sub_action->lexeme;
      if (lexeme) {
        if (lexeme->type == Lexeme::type_identifier) {
          if (lexeme->subtype == Lexeme::subtype_numeric) {
            context->variableEmitter->addVarAddress(sub_action);
            cpu.addPopDE();
            cpu.addLdiHLE();
            cpu.addIncHL();
            cpu.addLdiHLD();

          } else {
            context->syntaxError(
                "Invalid GET DATE parameter type.\nTry: GET DATE iYear, "
                "iMonth, iDay, iWeek, "
                "iDateFmt");
            return;
          }

        } else {
          context->syntaxError(
              "Invalid GET DATE parameter: it must be an integer variable.");
          return;
        }
      }
    }

  } else {
    context->syntaxError(
        "Wrong GET DATE parameters count.\nTry: GET DATE iYear, iMonth, "
        "iDay, "
        "iWeek, iDateFmt");
  }
}

void CompilerStatementEmitter::cmd_get_time() {
  auto& cpu = *context->cpu;
  Lexeme* lexeme;
  ActionNode *action = context->current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();

  if (t == 3) {
    cpu.addCall(def_get_time);
    cpu.addLdB(0);
    cpu.addLdCA();
    cpu.addPushBC();  // seconds
    cpu.addLdCL();
    cpu.addPushBC();  // minutes
    cpu.addLdCH();
    cpu.addPushBC();  // hours

    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      lexeme = sub_action->lexeme;
      if (lexeme) {
        if (lexeme->type == Lexeme::type_identifier) {
          if (lexeme->subtype == Lexeme::subtype_numeric) {
            context->variableEmitter->addVarAddress(sub_action);
            cpu.addPopDE();
            cpu.addLdiHLE();
            cpu.addIncHL();
            cpu.addLdiHLD();

          } else {
            context->syntaxError(
                "Invalid GET TIME parameter type.\nTry: GET TIME iHour, "
                "iMinute, iSecond");
            return;
          }

        } else {
          context->syntaxError(
              "Invalid GET TIME parameter: it must be an integer variable.");
          return;
        }
      }
    }

  } else {
    context->syntaxError(
        "Wrong GET TIME parameters count.\nTry: GET TIME iHour, iMinute, "
        "iSecond");
  }
}

void CompilerStatementEmitter::cmd_get_tile() {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
  ActionNode *action = context->current_action->actions[0], *sub_action;
  Lexeme *lexeme, *sub_lexeme;
  unsigned int t;
  int result_subtype;

  t = action->actions.size();
  if (t) {
    action = action->actions[0];
    lexeme = action->lexeme;
    t = action->actions.size();

    if (lexeme->value == "PATTERN") {
      if (t == 2) {
        // tile number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; tile number
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        cpu.addLdAL();
        cpu.addPushAF();

        // tile buffer pointer (8 bytes)
        sub_action = action->actions[1];
        sub_lexeme = sub_action->lexeme;
        if (sub_lexeme->type == Lexeme::type_identifier) {
          // ld hl, variable
          fixup.addFix(sub_lexeme);
          cpu.addLdHL(0x0000);
          result_subtype = Lexeme::subtype_numeric;
        } else {
          result_subtype = expression.evalExpression(sub_action);
        }
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        cpu.addPopAF();

        // call get_tile_pattern
        //   a = tile number
        //   hl = pointer to an 8 bytes buffer
        cpu.addCall(def_get_tile_pattern);

      } else {
        context->syntaxError(
            "Wrong parameters count on GET TILE PATTERN statement");
      }

    } else if (lexeme->value == "COLOR") {
      if (t == 2) {
        // tile number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; tile number
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        cpu.addLdAL();
        cpu.addPushAF();

        // tile buffer pointer (8 bytes)
        sub_action = action->actions[1];
        sub_lexeme = sub_action->lexeme;
        if (sub_lexeme->type == Lexeme::type_identifier) {
          // ld hl, variable
          fixup.addFix(sub_lexeme);
          cpu.addLdHL(0x0000);
          result_subtype = Lexeme::subtype_numeric;
        } else {
          result_subtype = expression.evalExpression(sub_action);
        }
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        cpu.addPopAF();

        // call get_tile_color
        //   a = sprite number
        //   hl = pointer to an 8 bytes buffer
        cpu.addCall(def_get_tile_color);

      } else {
        context->syntaxError(
            "Wrong parameters count on GET TILE COLOR statement");
      }

    } else {
      context->syntaxError("Invalid syntax on GET TILE statement");
    }
  } else {
    context->syntaxError("Missing parameters on GET TILE statement");
  }
}

void CompilerStatementEmitter::cmd_get_sprite() {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
  ActionNode *action = context->current_action->actions[0], *sub_action;
  Lexeme *lexeme, *sub_lexeme;
  unsigned int t;
  int result_subtype;

  t = action->actions.size();
  if (t) {
    action = action->actions[0];
    lexeme = action->lexeme;
    t = action->actions.size();

    if (lexeme->value == "PATTERN") {
      if (t == 2) {
        // sprite number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; sprite number
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        cpu.addLdAL();
        cpu.addPushAF();

        // sprite buffer pointer (32 bytes)
        sub_action = action->actions[1];
        sub_lexeme = sub_action->lexeme;
        if (sub_lexeme->type == Lexeme::type_identifier) {
          // ld hl, variable
          fixup.addFix(sub_lexeme);
          cpu.addLdHL(0x0000);
          result_subtype = Lexeme::subtype_numeric;
        } else {
          result_subtype = expression.evalExpression(sub_action);
        }
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        cpu.addPopAF();

        // call get_sprite_pattern
        //   a = sprite number
        //   hl = pointer to a 32 bytes buffer
        cpu.addCall(def_get_sprite_pattern);

      } else {
        context->syntaxError(
            "Wrong parameters count on GET SPRITE PATTERN statement");
      }

    } else if (lexeme->value == "COLOR") {
      if (t == 2) {
        // sprite number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; sprite number
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        cpu.addLdAL();
        cpu.addPushAF();

        // sprite buffer pointer (16 bytes)
        sub_action = action->actions[1];
        sub_lexeme = sub_action->lexeme;
        if (sub_lexeme->type == Lexeme::type_identifier) {
          // ld hl, variable
          fixup.addFix(sub_lexeme);
          cpu.addLdHL(0x0000);
          result_subtype = Lexeme::subtype_numeric;
        } else {
          result_subtype = expression.evalExpression(sub_action);
        }
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        cpu.addPopAF();

        // call get_sprite_color
        //   a = sprite number
        //   hl = pointer to a 16 bytes buffer
        cpu.addCall(def_get_sprite_color);

      } else {
        context->syntaxError(
            "Wrong parameters count on GET SPRITE COLOR statement");
      }

    } else {
      context->syntaxError("Invalid syntax on GET SPRITE statement");
    }
  } else {
    context->syntaxError("Missing parameters on GET SPRITE statement");
  }
}

void CompilerStatementEmitter::cmd_sprite() {
  auto& cpu = *context->cpu;
  ActionNode* action;
  Lexeme* next_lexeme;
  unsigned int t = context->current_action->actions.size();

  if (t == 1) {
    action = context->current_action->actions[0];
    next_lexeme = action->lexeme;

    if (next_lexeme->value == "LOAD") {
      context->current_action = action;
      cmd_sprite_load();
      return;
    }

    // ld hl, 0xFC6D   ; SPRITE state
    cpu.addLdHL(0xFC6D);

    if (next_lexeme->type == Lexeme::type_keyword &&
        next_lexeme->value == "ON") {
      // call 0x6c89   ; xbasic turn on trap (hl=trap state address)
      cpu.addCall(def_XBASIC_TRAP_ON);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "OFF") {
      // call 0x6c9c   ; xbasic turn off trap (hl=trap state address)
      cpu.addCall(def_XBASIC_TRAP_OFF);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "STOP") {
      // call 0x6ca5   ; xbasic turn stop trap (hl=trap state address)
      cpu.addCall(def_XBASIC_TRAP_STOP);
    } else {
      context->syntaxError("Invalid SPRITE statement");
    }

  } else {
    context->syntaxError("Empty SPRITE statement");
  }
}

void CompilerStatementEmitter::cmd_sprite_load() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode* action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t) {
    if (t > 1) {
      context->syntaxError("SPRITE LOAD with excess of parameters");
      return;
    }

    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      result_subtype = expression.evalExpression(action);
      expression.addCast(result_subtype, Lexeme::subtype_numeric);

      // ld (DAC), hl
      cpu.addLdiiHL(def_DAC);
      // call cmd_wrtspr                    ; tiny sprite loader
      cpu.addCall(def_cmd_wrtspr);
    }

  } else {
    context->syntaxError("SPRITE LOAD with empty parameters");
  }
}

void CompilerStatementEmitter::cmd_key() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode *action1, *action2;
  Lexeme* next_lexeme;
  unsigned int t = context->current_action->actions.size();
  int result_subtype;

  if (t == 1) {
    action1 = context->current_action->actions[0];
    next_lexeme = action1->lexeme;

    if (next_lexeme->type == Lexeme::type_keyword &&
        next_lexeme->value == "ON") {
      // call 0x00CF   ; DSPFNK - (0xF3DE = CNSDFG: function keys
      // presentation)
      cpu.addCall(0x00CF);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "OFF") {
      // call 0x00CC   ; ERAFNK
      cpu.addCall(0x00CC);
    } else {
      context->syntaxError("Invalid KEY statement");
    }

  } else if (t == 2) {
    action2 = context->current_action->actions[1];
    next_lexeme = action2->lexeme;

    // ld hl, key number
    action1 = context->current_action->actions[0];
    result_subtype = expression.evalExpression(action1);
    expression.addCast(result_subtype, Lexeme::subtype_numeric);

    // dec hl
    cpu.addDecHL();

    if ((next_lexeme->type == Lexeme::type_keyword &&
         next_lexeme->subtype == Lexeme::subtype_function) ||
        (next_lexeme->type == Lexeme::type_operator &&
         next_lexeme->value == "+") ||
        (next_lexeme->subtype == Lexeme::subtype_string)) {
      // add hl, hl
      cpu.addAddHLHL();
      // add hl, hl
      cpu.addAddHLHL();
      // add hl, hl
      cpu.addAddHLHL();
      // add hl, hl      ; key number * 16
      cpu.addAddHLHL();
      // ld de, 0xF87F   ; FNKSTR (10 x 16 bytes)
      cpu.addLdDE(0xf87f);
      // add hl, de
      cpu.addAddHLDE();

      // push hl
      cpu.addPushHL();
      //   ld a, 32        ; space
      cpu.addLdA(0x20);
      //   ld (hl), a
      cpu.addLdiHLA();
      //   ld e, l
      cpu.addLdEL();
      //   ld d, h
      cpu.addLdDH();
      //   inc de
      cpu.addIncDE();
      //   ld bc, 15
      cpu.addLdBC(0x000F);
      //   ldir
      cpu.addLDIR();

      //   ld hl, variable address
      result_subtype = expression.evalExpression(action2);
      expression.addCast(result_subtype, Lexeme::subtype_string);

      //   xor a
      cpu.addXorA();
      //   ld c, (hl)
      cpu.addLdCiHL();
      //   ld b, a
      cpu.addLdBA();
      //   inc hl
      cpu.addIncHL();

      // pop de
      cpu.addPopDE();

      // or c
      cpu.addOrC();
      // jr z, $+3
      cpu.addJrZ(0x02);
      //   ldir
      cpu.addLDIR();
      // ld (de), a
      cpu.addLdiDEA();
      // call 0x00C9    ; FNKSB
      cpu.addCall(0x00C9);

    } else {
      // ld e, l
      cpu.addLdEL();
      // ld d, h
      cpu.addLdDH();
      // add hl, de
      cpu.addAddHLDE();
      // add hl, de       ; key number * 3
      cpu.addAddHLDE();

      // ld de, 0xFC4C    ; KEY state position = key number * 3 + 0xFC4C
      cpu.addLdDE(0xfc4c);

      // add hl, de
      cpu.addAddHLDE();

      if (next_lexeme->type == Lexeme::type_keyword &&
          next_lexeme->value == "ON") {
        // call 0x6c89   ; xbasic turn on trap (hl=trap state address)
        cpu.addCall(def_XBASIC_TRAP_ON);
      } else if (next_lexeme->type == Lexeme::type_keyword &&
                 next_lexeme->value == "OFF") {
        // call 0x6c9c   ; xbasic turn off trap (hl=trap state address)
        cpu.addCall(def_XBASIC_TRAP_OFF);
      } else if (next_lexeme->type == Lexeme::type_keyword &&
                 next_lexeme->value == "STOP") {
        // call 0x6ca5   ; xbasic turn stop trap (hl=trap state address)
        cpu.addCall(def_XBASIC_TRAP_STOP);
      } else {
        context->syntaxError("Invalid KEY statement");
      }
    }

  } else {
    context->syntaxError("Empty KEY statement");
  }
}

void CompilerStatementEmitter::cmd_strig() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode *action1, *action2;
  Lexeme* next_lexeme;
  unsigned int t = context->current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    action2 = context->current_action->actions[1];
    next_lexeme = action2->lexeme;

    // ld hl, strig number
    action1 = context->current_action->actions[0];
    result_subtype = expression.evalExpression(action1);
    expression.addCast(result_subtype, Lexeme::subtype_numeric);

    // dec hl
    // cpuOpcodeWriter->addDecHL();

    // ld e, l
    cpu.addLdEL();
    // ld d, h
    cpu.addLdDH();
    // add hl, de
    cpu.addAddHLDE();
    // add hl, de       ; strig number * 3
    cpu.addAddHLDE();

    // ld de, 0xFC70    ; STRIG state position = key number * 3 + 0xFC70
    cpu.addLdDE(0xFC70);

    // add hl, de
    cpu.addAddHLDE();

    if (next_lexeme->type == Lexeme::type_keyword &&
        next_lexeme->value == "ON") {
      // call 0x6c89   ; xbasic turn on trap (hl=trap state address)
      cpu.addCall(def_XBASIC_TRAP_ON);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "OFF") {
      // call 0x6c9c   ; xbasic turn off trap (hl=trap state address)
      cpu.addCall(def_XBASIC_TRAP_OFF);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "STOP") {
      // call 0x6ca5   ; xbasic turn stop trap (hl=trap state address)
      cpu.addCall(def_XBASIC_TRAP_STOP);
    } else {
      context->syntaxError("Invalid STRIG statement");
    }

  } else {
    context->syntaxError("Wrong number of parameters in STRIG statement");
  }
}

void CompilerStatementEmitter::cmd_swap() {
  auto& cpu = *context->cpu;
  auto& optimizer = *context->codeOptimizer;
  Lexeme *lexeme1, *lexeme2;
  ActionNode *action1, *action2;
  unsigned int t = context->current_action->actions.size();

  if (t == 2) {
    action1 = context->current_action->actions[0];
    lexeme1 = action1->lexeme;
    if (lexeme1->type != Lexeme::type_identifier) {
      context->syntaxError("Invalid parameter type in SWAP (1st)");
      return;
    }

    action2 = context->current_action->actions[1];
    lexeme2 = action2->lexeme;
    if (lexeme2->type != Lexeme::type_identifier) {
      context->syntaxError("Invalid parameter type in SWAP (2nd)");
      return;
    }

    if (lexeme1->subtype == lexeme2->subtype) {
      // ld hl, variable
      context->variableEmitter->addVarAddress(action1);
      // push hl
      cpu.addPushHL();
      // ld hl, variable
      context->variableEmitter->addVarAddress(action2);
      // pop de
      optimizer.addByteOptimized(0xD1);

      if (lexeme1->subtype == Lexeme::subtype_numeric) {
        // call 0x6bf5    ; xbasic SWAP integers (in: hl=var1, de=var2)
        cpu.addCall(def_XBASIC_SWAP_INTEGER);
      } else if (lexeme1->subtype == Lexeme::subtype_string) {
        // call 0x6bf9    ; xbasic SWAP strings (in: hl=var1, de=var2)
        cpu.addCall(def_XBASIC_SWAP_STRING);
      } else {
        // call 0x6bfd    ; xbasic SWAP floats (in: hl=var1, de=var2)
        cpu.addCall(def_XBASIC_SWAP_FLOAT);
      }

    } else {
      context->syntaxError("Parameters type mismatch in SWAP");
    }

  } else {
    context->syntaxError("Invalid SWAP parameters");
  }
}

void CompilerStatementEmitter::cmd_wait() {
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

void CompilerStatementEmitter::cmd_data() {
  if (!context->current_action->actions.size()) {
    context->syntaxError("DATA with empty parameters");
  }
}

void CompilerStatementEmitter::cmd_idata() {
  if (!context->current_action->actions.size()) {
    context->syntaxError("IDATA with empty parameters");
  }
}

void CompilerStatementEmitter::cmd_read() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode* action;
  Lexeme* lexeme;
  unsigned int i, t = context->current_action->actions.size();

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->type != Lexeme::type_identifier) {
        context->syntaxError("Invalid READ parameter type");
        return;
      }

      // call read
      cpu.addCall(def_XBASIC_READ);

      expression.addCast(Lexeme::subtype_string, lexeme->subtype);

      // do assignment

      context->variableEmitter->addAssignment(action);
    }

  } else {
    context->syntaxError("READ with empty parameters");
  }
}

void CompilerStatementEmitter::cmd_iread() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode* action;
  Lexeme* lexeme;
  unsigned int i, t = context->current_action->actions.size();

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->type != Lexeme::type_identifier) {
        context->syntaxError("Invalid IREAD parameter type");
        return;
      }

      // call XBASIC_IREAD
      cpu.addCall(def_XBASIC_IREAD);

      expression.addCast(Lexeme::subtype_numeric, lexeme->subtype);

      // do assignment

      context->variableEmitter->addAssignment(action);
    }

  } else {
    context->syntaxError("IREAD with empty parameters");
  }
}

void CompilerStatementEmitter::cmd_resume() {
  context->syntaxError("RESUME statement not supported in compiled mode");
}

void CompilerStatementEmitter::cmd_restore() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode* action;
  Lexeme* lexeme;
  int t = context->current_action->actions.size();
  int result_subtype;

  if (t == 0) {
    // ld hl, 0
    cpu.addLdHL(0x0000);
    // call xbasic_restore
    cpu.addCall(def_XBASIC_RESTORE);  // standard BASIC RESTORE statement

  } else if (t == 1) {
    action = context->current_action->actions[0];

    lexeme = action->lexeme;
    if (lexeme) {
      result_subtype = expression.evalExpression(action);

      // cast
      expression.addCast(result_subtype, Lexeme::subtype_numeric);

      // call restore
      cpu.addCall(def_XBASIC_RESTORE);  // standard BASIC RESTORE statement
    }

  } else {
    context->syntaxError("RESTORE with wrong number of parameters");
  }
}

void CompilerStatementEmitter::cmd_irestore() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  ActionNode* action;
  Lexeme* lexeme;
  int t = context->current_action->actions.size();
  int result_subtype;

  if (t == 0) {
    // ld hl, (DATLIN)
    cpu.addLdHLii(def_DATLIN);
    // ld (DATPTR), hl
    cpu.addLdiiHL(def_DATPTR);

  } else if (t == 1) {
    action = context->current_action->actions[0];

    lexeme = action->lexeme;
    if (lexeme) {
      result_subtype = expression.evalExpression(action);

      // cast
      expression.addCast(result_subtype, Lexeme::subtype_numeric);

      // ex de, hl
      cpu.addExDEHL();
      // ld hl, (DATLIN)
      cpu.addLdHLii(def_DATLIN);
      // add hl, de
      cpu.addAddHLDE();
      // ld (DATPTR), hl
      cpu.addLdiiHL(def_DATPTR);
    }

  } else {
    context->syntaxError("IRESTORE with wrong number of parameters");
  }
}

void CompilerStatementEmitter::cmd_out() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        result_subtype = expression.evalExpression(action);

        // cast
        expression.addCast(result_subtype, Lexeme::subtype_numeric);

        if (i == 0) {
          // push hl
          cpu.addPushHL();
        } else {
          // pop bc
          cpu.addPopBC();
          // out (c), l
          cpu.addWord(0xED, 0x69);
        }
      }
    }

  } else {
    context->syntaxError("Invalid OUT parameters");
  }
}

void CompilerStatementEmitter::cmd_poke() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        result_subtype = expression.evalExpression(action);

        // cast
        expression.addCast(result_subtype, Lexeme::subtype_numeric);

        if (i == 0) {
          // push hl
          cpu.addPushHL();
        } else {
          // ex de, hl
          cpu.addExDEHL();
          // pop hl
          cpu.addPopHL();
          // ld (hl), e
          cpu.addLdiHLE();
        }
      }
    }

  } else {
    context->syntaxError("Invalid POKE parameters");
  }
}

void CompilerStatementEmitter::cmd_ipoke() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        result_subtype = expression.evalExpression(action);

        // cast
        expression.addCast(result_subtype, Lexeme::subtype_numeric);

        if (i == 0) {
          // push hl
          cpu.addPushHL();
        } else {
          // ex de, hl
          cpu.addExDEHL();
          // pop hl
          cpu.addPopHL();
          // ld (hl), e
          cpu.addLdiHLE();
          // inc hl
          cpu.addIncHL();
          // ld (hl), d
          cpu.addLdiHLD();
        }
      }
    }

  } else {
    context->syntaxError("Invalid IPOKE parameters");
  }
}

void CompilerStatementEmitter::cmd_vpoke() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        result_subtype = expression.evalExpression(action);

        // cast
        expression.addCast(result_subtype, Lexeme::subtype_numeric);

        if (i == 0) {
          // push hl
          cpu.addPushHL();
        } else {
          // ld a, l
          cpu.addLdAL();
          // pop hl
          cpu.addPopHL();
          // call 0x70b5    ; xbasic VPOKE (in: hl=address, a=byte)
          cpu.addCall(def_XBASIC_VPOKE);
        }
      }
    }

  } else {
    context->syntaxError("Invalid VPOKE parameters");
  }
}

void CompilerStatementEmitter::cmd_file() {
  auto& opts = *context->opts;
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int t = context->current_action->actions.size();
  string filename;

  if (t == 1) {
    action = context->current_action->actions[0];
    lexeme = action->lexeme;

    if (lexeme->type == Lexeme::type_literal &&
        lexeme->subtype == Lexeme::subtype_string) {
      lexeme->name = "FILE";
      filename = removeQuotes(lexeme->value);
      context->resourceManager.addFile(filename, opts.inputPath);
    } else {
      context->error_message = "Invalid parameter in FILE keyword";
    }

  } else {
    context->syntaxError("Wrong FILE parameters count");
  }
}

void CompilerStatementEmitter::cmd_text() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int t = context->current_action->actions.size();

  if (t == 1) {
    action = context->current_action->actions[0];
    lexeme = action->lexeme;

    if (lexeme->type == Lexeme::type_literal &&
        lexeme->subtype == Lexeme::subtype_string) {
      lexeme->name = "TEXT";
      context->resourceManager.addText(lexeme->value);
    } else {
      context->error_message = "Invalid parameter in TEXT keyword";
    }

  } else {
    context->syntaxError("Wrong TEXT parameters count");
  }
}

void CompilerStatementEmitter::cmd_call() {
  auto& cpu = *context->cpu;
  ActionNode* action;
  Lexeme* lexeme;
  unsigned int i, t = context->current_action->actions.size();

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->value == "TURBO") {
        // not need anymore... he he he...

      } else if (lexeme->value == "SYSTEM") {
        // ld hl, data address
        context->variableEmitter->addVarAddress(action);
        // ld a, (hl)      ; first character
        cpu.addLdAiHL();
        // ld ix, (CALL)    ; CALL
        cpu.addLdIXii(def_CALL_STMT);
        // call CALBAS
        cpu.addCall(def_CALBAS);
        // ei
        cpu.addEI();

      } else {
        // todo: concatenate CALL statement  into a literal string
        //       and run it with BASIC like above
        context->syntaxError("CALL statement invalid");
        return;
      }
    }

  } else {
    context->syntaxError("CALL with empty parameters");
  }
}

void CompilerStatementEmitter::cmd_open() {
  auto& cpu = *context->cpu;
  auto& opts = *context->opts;
  auto& expression = *context->expressionEvaluator;
  auto& optimizer = *context->codeOptimizer;
  auto& fixup = *context->fixupResolver;
  auto& codeHelper = *context->codeHelper;
  ActionNode* action;
  Lexeme* lexeme;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype, state = 0;
  FixNode* mark;
  bool has[4];

  for (i = 0; i < 4; i++) has[i] = false;

  context->file_support = true;

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;

      switch (state) {
        // file name
        case 0: {
          has[state] = true;
          state = 1;
          result_subtype = expression.evalExpression(action);
          expression.addCast(result_subtype, Lexeme::subtype_string);

          // push hl
          cpu.addPushHL();
        } break;

        // FOR
        case 1: {
          has[state] = true;
          state = 2;
          if (lexeme->value == "OUT") {
            // ld a, 2     ; output mode
            cpu.addLdA(0x02);
          } else if (lexeme->value == "APP") {
            // ld a, 8     ; append mode
            cpu.addLdA(0x08);
          } else if (lexeme->value == "INPUT") {
            // ld a, 1     ; input mode
            cpu.addLdA(0x01);
          } else {
            // ld a, 4     ; random mode
            cpu.addLdA(0x04);
            i--;
          }
          // push af
          cpu.addPushAF();
        } break;

        // AS
        case 2: {
          has[state] = true;
          state = 3;
          result_subtype = expression.evalExpression(action);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);
          // push hl
          cpu.addPushHL();
        } break;

        // LEN
        case 3: {
          has[state] = true;
          state = 4;

          result_subtype = expression.evalExpression(action);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);
        } break;
      }
    }

    codeHelper.addEnableBasicSlot();

    // LEN
    if (!has[3]) {
      // ld hl, 256  ; default record size
      cpu.addLdHL(0x0100);
    }
    // ld (RECSIZ), hl
    cpu.addLdiiHL(def_RECSIZ);

    // AS
    if (has[2]) {
      // pop hl
      cpu.addPopHL();
      // ld a, l
      cpu.addLdAL();
    } else {
      // xor a
      cpu.addXorA();
    }
    // ld (TEMP), a
    cpu.addLdiiA(def_TEMP);

    // FOR
    if (has[1]) {
      // pop af
      cpu.addPopAF();
    } else {
      // xor a
      cpu.addXorA();
    }
    // ld (TEMP+1), a           ; file mode
    cpu.addLdiiA(def_TEMP + 1);

    if (opts.megaROM) {
      // ld hl, 0x0000             ; get return point address
      mark = fixup.addMark();
      optimizer.addLdHLmegarom();
      // ld c, l
      cpu.addLdCL();
      // ld b, h
      cpu.addLdBH();
    } else {
      // ld bc, 0x0000             ; get return point address
      mark = fixup.addMark();
      cpu.addLdBC(0x0000);
    }

    // FILE
    if (has[0]) {
      // pop hl
      cpu.addPopHL();
    } else {
      // ld hl, NULL STR
      cpu.addLdHL(0x8000);
    }

    // push bc            ; save return point to run after FILESPEC
    cpu.addPushBC();

    // ld e, (hl)
    cpu.addLdEiHL();

    // inc hl
    cpu.addIncHL();

    // push hl
    cpu.addPushHL();

    // jp FILESPEC        ; in e=string size, hl=string start; out d = device
    // code, FILNAM
    cpu.addJp(def_FILESPEC);

    mark->symbol->address =
        cpu.context->code_pointer;  // return point after FILESPEC

    // ld a, (TEMP+1)
    cpu.addLdAii(def_TEMP + 1);
    // ld e, a                ; file mode
    cpu.addLdEA();

    // ld a, (TEMP)           ; io number
    cpu.addLdAii(def_TEMP);

    // ld hl, fake empty line
    cpu.addLdHL(def_ENDPRG);

    // call OPEN     ; in: a = i/o number, e = filemode, d = devicecode
    cpu.addCall(def_OPEN);

    codeHelper.addDisableBasicSlot();

  } else {
    context->syntaxError("Empty OPEN statement");
  }
}

void CompilerStatementEmitter::cmd_close() {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& codeHelper = *context->codeHelper;
  ActionNode* action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      result_subtype = expression.evalExpression(action);
      expression.addCast(result_subtype, Lexeme::subtype_numeric);

      // ld a, l
      cpu.addLdAL();
      // push af
      cpu.addPushAF();

      codeHelper.addEnableBasicSlot();

      // pop af
      cpu.addPopAF();
      // ld hl, fake empty line
      cpu.addLdHL(def_ENDPRG);

      // call CLOSE    ; in: a = i/o number
      cpu.addCall(def_CLOSE);

      codeHelper.addDisableBasicSlot();
    }

  } else {
    codeHelper.addEnableBasicSlot();

    // ld hl, fake empty line
    cpu.addLdHL(def_ENDPRG);

    // call ClOSE ALL
    cpu.addCall(def_CLOSE_ALL);

    codeHelper.addDisableBasicSlot();
  }
}

void CompilerStatementEmitter::cmd_cmd() {
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

//-------------------------------------------------------------------------------------------
