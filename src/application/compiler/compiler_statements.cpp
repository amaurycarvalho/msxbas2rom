/***
 * @file compiler_statements.cpp
 * @brief Compiler statement emission and related helper routines
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler.h"

#include "compiler_hooks.h"

void Compiler::cmd_start() {
  // ld (SAVSTK), sp
  addLdiiSP(0xf6b1);

  // ld a, (SLTSTR)     ; start slot
  addLdAii(def_SLTSTR);

  // ld h, 0x40
  addLdH(0x40);

  // call ENASLT        ; enable xbasic page
  addCall(def_ENASLT);

  // ld hl, HEAP START ADDRESS
  addFix(heap_mark);
  addLdHL(0x0000);

  // ld de, TEMPORARY STRING START ADDRESS
  addFix(temp_str_mark);
  addLdDE(0x0000);

  if (parser->getHasFont()) {
    // ld ix, FONT BUFFER START ADDRESS
    addByte(0xDD);
    addFix(heap_mark)->step = -def_RAM_BUFSIZ;
    addLdHL(0x0000);
  } else {
    // push hl
    addPushHL();
    // pop ix
    addPopIX();
  }

  // call XBASIC INIT                  ; hl=heap start address, de=temporary
  // string start address, bc=data address, ix=font address, a=data segment
  addCall(def_XBASIC_INIT);

  if (parser->getHasTraps()) {
    if (opts->megaROM) {
      // ld a, 0xFF
      addLdA(0xFF);
    } else {
      // xor a
      addXorA();
    }
    // ld (0xFC82), a          ; start of TRPTBL reserved area (megaROM flag to
    // traps)
    addLdiiA(def_MR_TRAP_FLAG);
  }

  if (opts->megaROM) {
    // ld a, 2
    addLdA(0x02);
    // call MR_CHANGE_SGM
    addCall(def_MR_CHANGE_SGM);
  }

  if (parser->getHasData() || parser->getHasIData()) {
    // ld hl, data resource number
    addLdHL(parser->getResourceCount());
    // ld (DAC), hl
    addLdiiHL(def_DAC);
    // call cmd_restore
    addCall(def_cmd_restore);  // MSXBAS2ROM resource RESTORE statement
  }

  if (parser->getHasAkm()) {
    // initialize AKM player
    addCall(def_player_initialize);
  }

  // ei
  addEI();
}

void Compiler::cmd_end(bool doCodeRegistering) {
  if (doCodeRegistering) {
    /// @remark first instruction needs to be a skip to the program start code
    if (parser->getHasAkm()) {
      addJr(1 + 3 + 10);
    } else
      addJr(1 + 10);

    /// mark the END statement start code
    end_mark = addPreMark();
    end_mark->address = code_pointer;

    /// write the END statement code
    if (parser->getHasAkm()) {
      // disable AKM player
      addCall(def_player_unhook);
    }

    // call XBASIC_END
    addCall(def_XBASIC_END);

    // ld sp, (SAVSTK)
    addLdSPii(0xf6b1);

    // ld hl, fake empty line
    addLdHL(def_ENDPRG);

    // ret               ; return to basic
    addRet();

  } else {
    /// jump to the real END statement
    // jp end_mark
    if (end_mark) {
      addFix(end_mark);
      addJp(0x0000);
    }
  }
}

void Compiler::cmd_cls() {
  addCall(def_XBASIC_CLS);  // call cls
}

void Compiler::cmd_clear() {
  /// clear variables workarea
  addXorA();                //! A = 0
  addLdHLii(def_HEAPSTR);   //! HL = heap start address
  addLdDE(def_RAM_BOTTOM);  //! DE = variables start address
  addSbcHLDE();             //! HL -= DE
  addLdCL();
  addLdBH();  //! BC = HL (variables workarea size)
  addLdLE();
  addLdHD();    //! HL = DE
  addIncDE();   //! DE = HL + 1
  addLdiHLA();  //! (HL) = A
  addLDIR();    //! (DE++) = (HL++), until BC-- = 0
}

void Compiler::cmd_beep() {
  addCall(0x00c0);  // call beep
}

void Compiler::cmd_randomize() {
  // ld hl, 0x3579      ; RANDOMIZE 1 - FIX
  addLdHL(0x3579);
  // ld (0xF7BC+0), hl  ; SWPTMP+0
  addLdiiHL(0xF7BC);

  // ld hl, (0xFC9E)    ; RANDOMIZE 2 - JIFFY
  addLdHLii(0xFC9E);
  // ld (0xF7BC+2), hl  ; SWPTMP+2
  addLdiiHL(0xF7BE);
}

void Compiler::cmd_goto() {
  Lexeme* lexeme;

  if (current_action->actions.size() == 1) {
    lexeme = current_action->actions[0]->lexeme;
    if (lexeme) {
      if (lexeme->type == Lexeme::type_literal &&
          lexeme->subtype == Lexeme::subtype_numeric) {
        // Trim leading zeros
        while (lexeme->value.find("0") == 0 && lexeme->value.size() > 1) {
          lexeme->value.erase(0, 1);
        }

        // jp address
        addFix(lexeme->value);
        addJp(0x0000);
        return;
      }
    }
  }

  syntaxError("Invalid GOTO parameters");
}

void Compiler::cmd_gosub() {
  Lexeme* lexeme;

  if (current_action->actions.size() == 1) {
    lexeme = current_action->actions[0]->lexeme;
    if (lexeme) {
      if (lexeme->type == Lexeme::type_literal &&
          lexeme->subtype == Lexeme::subtype_numeric) {
        // Trim leading zeros
        while (lexeme->value.find("0") == 0 && lexeme->value.size() > 1) {
          lexeme->value.erase(0, 1);
        }

        // call address
        addFix(lexeme->value);
        addCall(0x0000);
        return;
      }
    }
  }

  syntaxError("Invalid GOSUB parameters");
}

void Compiler::cmd_return() {
  Lexeme* lexeme;
  int t = current_action->actions.size();

  if (t == 0) {
    // ret
    addRet();
    return;

  } else if (t == 1) {
    lexeme = current_action->actions[0]->lexeme;
    if (lexeme) {
      if (lexeme->type == Lexeme::type_literal &&
          lexeme->subtype == Lexeme::subtype_numeric) {
        // Trim leading zeros
        while (lexeme->value.find("0") == 0 && lexeme->value.size() > 1) {
          lexeme->value.erase(0, 1);
        }

        if (opts->megaROM) {
          // pop bc           ; delete old return segment/address
          addPopBC();
          // pop de           ; delete old return segment/address
          addPopDE();
          // ld a, 0x48       ; verify if running on trap (MR_CALL_TRAP)
          addLdA(0x48);
          // cp b
          addCpB();
          // jp nz, address   ; if not, jump to new address and segment
          addFix(lexeme->value);
          addJpNZ(0x0000);

          // pop de           ; fix trap return control
          addPopDE();
          // pop hl
          addPopHL();
          // exx
          addExx();
          // special ld hl, address
          addFix(lexeme->value);
          addLdHLmegarom();
          // ex (sp), hl      ; new return address
          addExiSPHL();
          // exx
          addExx();
          // push hl
          addPushHL();
          // push de
          addPushDE();
          // push af          ; new return segment
          addPushAF();
          // push bc          ; trap return
          addPushBC();
          // ret
          addRet();

        } else {
          // pop bc           ; delete old return address
          addPopBC();
          // ld a, 0x6C       ; verify if running on trap
          addLdA(0x6C);
          // cp b
          addCpB();
          // jp nz, address   ; jump to new address
          addFix(lexeme->value);
          addJpNZ(0x0000);

          // pop de           ; fix trap return control
          addPopDE();
          // ld hl, address
          addFix(lexeme->value);
          addLdHL(0x0000);
          // ex (sp), hl
          addExiSPHL();
          // push de
          addPushDE();
          // push bc
          addPushBC();
          // ret
          addRet();
        }

        return;
      }
    }
  }

  syntaxError("Invalid RETURN parameters");
}

void Compiler::cmd_print() {
  Lexeme *lexeme, *last_lexeme = 0;
  ActionNode *action, *subaction;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;
  bool redirected = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;
      last_lexeme = lexeme;

      if (lexeme) {
        if (lexeme->type == Lexeme::type_separator) {
          if (lexeme->value == ",") {
            addCall(def_XBASIC_PRINT_TAB);  // call print_tab
          } else if (lexeme->value == ";") {
            continue;
          } else if (lexeme->value == "#") {
            if (has_open_grp) continue;

            redirected = true;
            subaction = action->actions[0];
            result_subtype = evalExpression(subaction);
            addCast(result_subtype, Lexeme::subtype_numeric);

            // call io redirect
            if (io_redirect_mark)
              addFix(io_redirect_mark->symbol);
            else
              io_redirect_mark = addMark();
            addCall(0x0000);

            continue;
          } else {
            syntaxError("Invalid PRINT parameter separator");
            return;
          }
        } else {
          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_string) {
            addCall(def_XBASIC_PRINT_STR);  // call print_str

          } else if (result_subtype == Lexeme::subtype_numeric) {
            addCall(def_XBASIC_PRINT_INT);  // call print_int

          } else if (result_subtype == Lexeme::subtype_single_decimal ||
                     result_subtype == Lexeme::subtype_double_decimal) {
            addCall(def_XBASIC_PRINT_FLOAT);  // call print_float

          } else {
            syntaxError("Invalid PRINT parameter");
            return;
          }
        }
      }
    }

  } else {
    addCall(def_XBASIC_PRINT_CRLF);  // call print_crlf
  }

  if (last_lexeme) {
    if (last_lexeme->type != Lexeme::type_separator ||
        (last_lexeme->value != ";" && last_lexeme->value != ",")) {
      addCall(def_XBASIC_PRINT_CRLF);  // call print_crlf
    }
  }

  if (redirected) {
    // call io screen
    if (io_screen_mark)
      addFix(io_screen_mark->symbol);
    else
      io_screen_mark = addMark();
    addCall(0x0000);
  }
}

void Compiler::cmd_input(bool question) {
  Lexeme* lexeme;
  ActionNode *action, *subaction;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;
  bool redirected = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        if (lexeme->type == Lexeme::type_separator) {
          if (lexeme->value == ",") {
            addCall(def_XBASIC_PRINT_TAB);  // call print_tab
          } else if (lexeme->value == ";") {
            continue;
          } else if (lexeme->value == "#") {
            redirected = true;
            subaction = action->actions[0];
            result_subtype = evalExpression(subaction);
            addCast(result_subtype, Lexeme::subtype_numeric);

            // call io redirect
            if (io_redirect_mark)
              addFix(io_redirect_mark->symbol);
            else
              io_redirect_mark = addMark();
            addCall(0x0000);

            continue;
          } else {
            syntaxError("Invalid INPUT parameter separator");
            return;
          }
        } else {
          if (lexeme->type == Lexeme::type_identifier) {
            // call INPUT or LINE INPUT

            if (question) {
              addCall(def_XBASIC_INPUT_1);
            } else {
              addCall(def_XBASIC_INPUT_2);
            }

            // do assignment

            addCast(Lexeme::subtype_string, lexeme->subtype);

            if (!addAssignment(action)) return;

          } else {
            result_subtype = evalExpression(action);

            if (result_subtype == Lexeme::subtype_string) {
              addCall(def_XBASIC_PRINT_STR);  // call print_str

            } else if (result_subtype == Lexeme::subtype_numeric) {
              addCall(def_XBASIC_PRINT_INT);  // call print_int

            } else if (result_subtype == Lexeme::subtype_single_decimal ||
                       result_subtype == Lexeme::subtype_double_decimal) {
              addCall(def_XBASIC_PRINT_FLOAT);  // call print_float

            } else {
              syntaxError("Invalid INPUT parameter");
              return;
            }
          }
        }
      }
    }

    if (redirected) {
      // call io screen
      if (io_screen_mark)
        addFix(io_screen_mark->symbol);
      else
        io_screen_mark = addMark();
      addCall(0x0000);
    }

  } else {
    syntaxError();
  }
}

void Compiler::cmd_line() {
  Lexeme* lexeme;
  ActionNode *action, *sub_action;
  unsigned int i, t = current_action->actions.size();
  int line_type = 0, result_subtype, state;
  bool has_x0_coord = false, has_x1_coord = false, has_y0_coord = false,
       has_y1_coord = false;
  bool has_color = false, has_line_type = false, has_operator = false;

  if (t) {
    action = current_action->actions[0];
    lexeme = action->lexeme;
    if (lexeme->type == Lexeme::type_keyword && lexeme->value == "INPUT") {
      current_action = action;
      return cmd_input(false);
    }

    state = 0;

    for (i = 0; i < t; i++) {
      action = current_action->actions[i];

      switch (state) {
        case 0: {
          if (action->actions.size() != 2) {
            syntaxError("Coordenates parameters error on LINE");
            return;
          } else if (action->lexeme->value == "COORD") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
              // ld hl, (0xFCB7)  ;GRPACX
              addLdHLii(0xFCB7);
              // push hl
              addPushHL();
              has_x0_coord = true;
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();
              has_x0_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
              // ld hl, (0xFCB9)  ;GRPACY
              addLdHLii(0xFCB9);
              // push hl
              addPushHL();
              has_y0_coord = true;
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();
              has_y0_coord = true;
            }

          } else if (action->lexeme->value == "STEP") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
              // ld hl, (0xFCB7)  ;GRPACX
              addLdHLii(0xFCB7);
              // push hl
              addPushHL();
              has_x0_coord = true;
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB7)  ;GRPACX
              addLdDEii(0xFCB7);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();
              has_x0_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
              // ld hl, (0xFCB9)  ;GRPACY
              addLdHLii(0xFCB9);
              // push hl
              addPushHL();
              has_y0_coord = true;
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB9)  ;GRPACY
              addLdDEii(0xFCB9);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();
              has_y0_coord = true;
            }

          } else if (action->lexeme->value == "TO_COORD") {
            state++;

            if (!has_x0_coord) {
              // ex de,hl
              addExDEHL();
              // ld hl, (0xFCB7)  ;GRPACX
              addLdHLii(0xFCB7);
              // push hl
              addPushHL();
              // ld hl, (0xFCB9)  ;GRPACY
              addLdHLii(0xFCB9);
              // push hl
              addPushHL();
              // ex de,hl
              addExDEHL();
              has_x0_coord = true;
              has_y0_coord = true;
            }

            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();
              has_x1_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();
              has_y1_coord = true;
            }

          } else if (action->lexeme->value == "TO_STEP") {
            state++;

            if (!has_x0_coord) {
              // ex de,hl
              addExDEHL();
              // ld hl, (0xFCB7)  ;GRPACX
              addLdHLii(0xFCB7);
              // push hl
              addPushHL();
              // ld hl, (0xFCB9)  ;GRPACY
              addLdHLii(0xFCB9);
              // push hl
              addPushHL();
              // ex de,hl
              addExDEHL();
              has_x0_coord = true;
              has_y0_coord = true;
            }

            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB7)  ;GRPACX
              addLdDEii(0xFCB7);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();
              has_x1_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB9)  ;GRPACY
              addLdDEii(0xFCB9);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();
              has_y1_coord = true;
            }

          } else {
            syntaxError("Invalid coordenates on LINE");
            return;
          }

        } break;

        case 1: {
          state++;

          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();

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
            syntaxError("Invalid shape parameter");
          }

          has_line_type = true;

        } break;

        case 3: {
          state++;

          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld b, l
          addLdBL();

          has_operator = true;

        } break;

        default: {
          syntaxError("LINE parameters not supported");
          return;
        }
      }
    }

    if (!has_color) {
      // ld a, (0xF3E9)       ; FORCLR
      addLdAii(0xF3E9);
    }

    if (!has_operator) {
      // ld b, 0
      addLdB(0x00);
    }

    if (has_y1_coord) {
      // pop hl
      addPopHL();
    } else {
      // ld hl, (0xFCB9)  ;GRPACY
      addLdHLii(0xFCB9);
    }

    if (has_x1_coord) {
      // pop de
      addPopDE();
    } else {
      // ld de, (0xFCB7)  ;GRPACX
      addLdDEii(0xFCB7);
    }

    if (has_y0_coord) {
      // pop iy
      addPopIY();
    } else {
      // ld iy, (0xFCB9)  ;GRPACY
      addLdIYii(0xFCB9);
    }

    if (has_x0_coord) {
      // pop ix
      addPopIX();
    } else {
      // ld ix, (0xFCB7)  ;GRPACX
      addLdIXii(0xFCB7);
    }

    if (line_type == 0 || !has_line_type) {
      // call 0x6DA7   ; xbasic LINE (in: ix=x0, iy=y0, de=x1, hl=y1, a=color,
      // b=operator)
      addCall(def_XBASIC_LINE);
    } else if (line_type == 1) {
      // call 0x6D49   ; xbasic BOX (in: ix=x0, iy=y0, de=x1, hl=y1, a=color,
      // b=operator)
      addCall(def_XBASIC_BOX);
    } else {
      // call 0x6E27   ; xbasic BOX FILLED (in: ix=x0, iy=y0, de=x1, hl=y1,
      // a=color, b=operator)
      addCall(def_XBASIC_BOXF);
    }

  } else {
    syntaxError("LINE with empty parameters");
  }
}

void Compiler::cmd_copy() {
  ActionNode *action, *sub_action;
  Lexeme* lexeme;
  unsigned int i, t = current_action->actions.size();
  int result_subtype, state;
  bool has_x0_coord = false, has_x1_coord = false, has_x2_coord = false;
  bool has_y0_coord = false, has_y1_coord = false, has_y2_coord = false;
  bool has_src_page = false, has_dest_page = false, has_operator = false;
  bool has_address_from = false, has_address_to = false;

  if (t) {
    // action = current_action->actions[0];

    state = 0;

    for (i = 0; i < t; i++) {
      action = current_action->actions[i];

      switch (state) {
        case 0: {
          if (action->lexeme->value == "SCREEN") {
            return cmd_copy_screen();
          } else if (action->lexeme->value == "COORD") {
            if (action->actions.size() != 2) {
              syntaxError("Coordenates parameters error on COPY");
              return;
            }

            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
              // ld hl, (0xFCB7)  ;GRPACX
              addLdHLii(0xFCB7);
              // push hl
              addPushHL();
              has_x0_coord = true;
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();
              has_x0_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
              // ld hl, (0xFCB9)  ;GRPACY
              addLdHLii(0xFCB9);
              // push hl
              addPushHL();
              has_y0_coord = true;
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();
              has_y0_coord = true;
            }

          } else if (action->lexeme->value == "STEP") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
              // ld hl, (0xFCB7)  ;GRPACX
              addLdHLii(0xFCB7);
              // push hl
              addPushHL();
              has_x0_coord = true;
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB7)  ;GRPACX
              addLdDEii(0xFCB7);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();
              has_x0_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
              // ld hl, (0xFCB9)  ;GRPACY
              addLdHLii(0xFCB9);
              // push hl
              addPushHL();
              has_y0_coord = true;
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB9)  ;GRPACY
              addLdDEii(0xFCB9);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();
              has_y0_coord = true;
            }

          } else if (action->lexeme->value == "TO_COORD") {
            state++;

            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();
              has_x1_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();
              has_y1_coord = true;
            }

          } else if (action->lexeme->value == "TO_STEP") {
            state++;

            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB7)  ;GRPACX
              addLdDEii(0xFCB7);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();
              has_x1_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB9)  ;GRPACY
              addLdDEii(0xFCB9);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();
              has_y1_coord = true;
            }

          } else {
            // copy from RAM address to vram
            has_address_from = true;

            if ((lexeme = action->lexeme)) {
              if (lexeme->type == Lexeme::type_identifier && lexeme->isArray) {
                // ld hl, variable
                addFix(lexeme);
                addLdHL(0x0000);
                result_subtype = Lexeme::subtype_numeric;
              } else {
                result_subtype = evalExpression(action);
              }
            } else {
              result_subtype = evalExpression(action);
            }

            if (result_subtype == Lexeme::subtype_null ||
                result_subtype == Lexeme::subtype_single_decimal ||
                result_subtype == Lexeme::subtype_double_decimal) {
              syntaxError("Invalid address in COPY");
              return;
            }

            // push hl
            addPushHL();

            state++;
          }

        } break;

        case 1: {
          state++;

          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();
          // push af
          addPushAF();

          has_src_page = true;

        } break;

        case 2: {
          if (action->lexeme->value == "TO_DEST") {
            state++;

            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();
              has_x2_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();
              has_y2_coord = true;
            }

          } else {
            // copy from VRAM to RAM address
            has_address_to = true;

            if ((lexeme = action->lexeme)) {
              if (lexeme->type == Lexeme::type_identifier && lexeme->isArray) {
                // ld hl, variable
                addFix(lexeme);
                addLdHL(0x0000);
                result_subtype = Lexeme::subtype_numeric;
              } else {
                result_subtype = evalExpression(action);
              }
            } else {
              result_subtype = evalExpression(action);
            }

            if (result_subtype == Lexeme::subtype_null ||
                result_subtype == Lexeme::subtype_single_decimal ||
                result_subtype == Lexeme::subtype_double_decimal) {
              syntaxError("Invalid address in COPY");
              return;
            }

            state = 99;  // exit loop
          }

        } break;

        case 3: {
          state++;

          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();
          // push af
          addPushAF();

          has_dest_page = true;

        } break;

        case 4: {
          state++;

          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();
          addPushAF();

          has_operator = true;

        } break;

        default: {
          syntaxError("COPY parameters not supported");
          return;
        }
      }
    }

    if (has_address_from) {
      if (!has_operator) {
        // xor a
        addXorA();
      } else {
        addPopAF();
      }
      // ld b, a
      addLdBA();
      // ld (LOGOP), a
      addLdiiA(def_LOGOP);

      if (has_dest_page) {
        // pop af
        addPopAF();
        // ld (ACPAGE), a
        addLdiiA(def_ACPAGE);
      }

      if (has_y2_coord) {
        // pop hl
        addPopHL();
      } else {
        // ld hl, (0xFCB9)  ;GRPACY
        addLdHLii(0xFCB9);
      }
      // ld (DY), hl
      addLdiiHL(def_DY);

      if (has_x2_coord) {
        // pop hl
        addPopHL();
      } else {
        // ld hl, (0xFCB7)  ;GRPACX
        addLdHLii(0xFCB7);
      }
      // ld (DX), hl
      addLdiiHL(def_DX);

      if (has_src_page) {
        // pop af
        addPopAF();
        // add a,a
        addAddA();
        // add a,a
        addAddA();
        // ld (ARGT), a    ; direction/expansion (0000DDEE)
        addLdiiA(def_ARGT);
      }

      // pop hl
      addPopHL();

      // call XBASIC_COPY_FROM
      addCall(def_XBASIC_COPY_FROM);

    } else if (has_address_to) {
      // ex de,hl      ; address to (hl to de)
      addExDEHL();

      if (has_src_page) {
        // pop af
        addPopAF();
        // ld (ACPAGE), a
        addLdiiA(def_ACPAGE);
      }

      if (has_y1_coord) {
        // pop hl
        addPopHL();
      } else {
        // ld hl, (0xFCB9)  ;GRPACY
        addLdHLii(0xFCB9);
      }
      // ld (NY), hl
      addLdiiHL(def_NY);

      if (has_x1_coord) {
        // pop hl
        addPopHL();
      } else {
        // ld hl, (0xFCB7)  ;GRPACX
        addLdHLii(0xFCB7);
      }
      // ld (NX), hl
      addLdiiHL(def_NX);

      if (has_y0_coord) {
        // pop hl
        addPopHL();
      } else {
        // ld hl, (0xFCB9)  ;GRPACY
        addLdHLii(0xFCB9);
      }
      // ld (SY), hl
      addLdiiHL(def_SY);

      if (has_x0_coord) {
        // pop hl
        addPopHL();
      } else {
        // ld hl, (0xFCB7)  ;GRPACX
        addLdHLii(0xFCB7);
      }
      // ld (SX), hl
      addLdiiHL(def_SX);

      // ex de,hl
      addExDEHL();

      // call XBASIC_COPY_TO
      addCall(def_XBASIC_COPY_TO);

    } else {
      if (!has_operator) {
        // ld b, 0
        addLdB(0x00);
      } else {
        addPopBC();
      }

      if (has_dest_page) {
        // pop af
        addPopAF();
      } else {
        // ld a, (ACPAGE)
        addLdAii(def_ACPAGE);
      }
      // ld (0xFC19), a
      addLdiiA(0xFC19);

      if (has_y2_coord) {
        // pop hl
        addPopHL();
        // ld (0xFCB9), hl  ;GRPACY
        addLdiiHL(0xFCB9);
      }

      if (has_x2_coord) {
        // pop hl
        addPopHL();
        // ld (0xFCB7), hl  ;GRPACX
        addLdiiHL(0xFCB7);
      }

      if (has_src_page) {
        // pop af
        addPopAF();
      } else {
        // ld a, (ACPAGE)
        addLdAii(def_ACPAGE);
      }
      // ld (0xFC18), a
      addLdiiA(0xFC18);

      if (has_y1_coord) {
        // pop iy
        addPopIY();
      } else {
        // ld iy, (0xFCB9)  ;GRPACY
        addLdIYii(0xFCB9);
      }

      if (has_x1_coord) {
        // pop ix
        addPopIX();
      } else {
        // ld ix, (0xFCB7)  ;GRPACX
        addLdIXii(0xFCB7);
      }

      if (has_y0_coord) {
        // pop hl
        addPopHL();
      } else {
        // ld hl, (0xFCB9)  ;GRPACY
        addLdHLii(0xFCB9);
      }

      if (has_x0_coord) {
        // pop de
        addPopDE();
      } else {
        // ld de, (0xFCB7)  ;GRPACX
        addLdDEii(0xFCB7);
      }

      // call COPY    ; in: de=x0, hl=y0, ix=x1, iy=y1, 0xFC18=srcpg, 0xFCB7=x2,
      // 0xFCB9=y2, 0xFC19=destpg, b=operator
      addCall(def_XBASIC_COPY);
    }

  } else {
    syntaxError("COPY with empty parameters");
  }
}

void Compiler::cmd_copy_screen() {
  int t = current_action->actions.size();

  if (t > 1) {
    syntaxError("Invalid COPY SCREEN parameters");
  } else {
    if (t == 0) {
      // xor a
    } else if (t == 1) {
      ActionNode *action = current_action->actions[0], *sub_action;
      int result_subtype;

      sub_action = action->actions[0];
      result_subtype = evalExpression(sub_action);

      addCast(result_subtype, Lexeme::subtype_numeric);

      // ld a, l
      addLdAL();
    }

    // and 1
    addAnd(0x01);
    // inc a
    addIncA();
    // or 0x10
    addOr(0x10);
    // ld hl, BUF
    addLdHL(def_BUF);
    // push hl
    addPushHL();
    //   ld (hl), 0xC5  ; SCREEN token
    addLdiHL(0xC5);
    //   inc hl
    addIncHL();
    //   ld (hl), a
    addLdiHLA();
    //   inc hl
    addIncHL();
    //   xor a
    addXorA();
    //   ld (hl), a
    addLdiHLA();
    //   inc hl
    addIncHL();
    //   ld (hl), a
    addLdiHLA();
    // pop hl
    addPopHL();

    // ld a, (VERSION)
    addLdAii(def_VERSION);
    // and a
    addAndA();
    // jr z, skip
    addJrZ(9);

    //   ld a, (hl)       ; first character
    addLdAiHL();
    //   ld ix, (COPY)    ; COPY
    addLdIXii(def_COPY_STMT);
    //   call CALBAS
    addCall(def_CALBAS);
    //   ei
    addEI();

    // skip:
  }

  return;
}

void Compiler::cmd_sound() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        result_subtype = evalExpression(action);

        // cast
        addCast(result_subtype, Lexeme::subtype_numeric);

        if (i == 0) {
          // ld a, l
          addLdAL();
          // push af
          addPushAF();
        } else {
          // ld e, l
          addLdEL();
          // pop af
          addPopAF();

          // call sound function
          addCall(def_XBASIC_SOUND);
        }
      }
    }

  } else {
    syntaxError("Invalid SOUND parameters");
  }
}

void Compiler::cmd_bload() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int t = current_action->actions.size();
  bool isTinySprite;
  int resource_number;
  string filename, fileext;

  if (t == 2) {
    action = current_action->actions[1];
    lexeme = action->lexeme;
    if (lexeme) {
      if (lexeme->name == "S") {
        action = current_action->actions[0];
        lexeme = action->lexeme;
        if (lexeme) {
          if (lexeme->type == Lexeme::type_literal &&
              lexeme->subtype == Lexeme::subtype_string) {
            // add to resource list

            // verify file type (screen or sprite)

            filename = removeQuotes(lexeme->value);
            fileext = getFileExtension(filename);
            isTinySprite = (strcasecmp((char*)fileext.c_str(), ".SPR") == 0);

            resource_number = resourceManager.resources.size();
            lexeme->name = "FILE";
            resourceManager.addFile(filename, opts->inputPath);

            // execute a resource screen load

            // ld hl, resource number
            addLdHL(resource_number);

            // ld (DAC), hl
            addLdiiHL(def_DAC);

            if (isTinySprite) {
              // call cmd_wrtspr                    ; tiny sprite loader
              addCall(def_cmd_wrtspr);
            } else {
              // call screen_load
              addCall(def_cmd_screen_load);
            }

          } else {
            syntaxError("BLOAD 1st parameter must be a string constant");
          }

        } else {
          syntaxError("BLOAD 1st parameter error");
        }

      } else {
        syntaxError("BLOAD valid only to screen");
      }

    } else {
      syntaxError("BLOAD 2nd parameter error");
    }

  } else {
    syntaxError("Invalid BLOAD parameters count");
  }
}

void Compiler::cmd_play() {
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;
  bool xor_a = true;

  if (t >= 1 && t <= 3) {
    for (i = 0; i < 3; i++) {
      if (i < t) {
        action = current_action->actions[i];
        result_subtype = evalExpression(action);
        if (result_subtype != Lexeme::subtype_string) {
          syntaxError("Invalid PLAY parameter");
          return;
        }
        // push hl
        addPushHL();
      } else {
        if (xor_a) {
          // xor a
          addXorA();
          xor_a = false;
        }
        // push af
        addPushAF();
      }
    }

    // pop bc
    addPopBC();
    // pop de
    addPopDE();
    // pop hl
    addPopHL();

    addCall(def_XBASIC_PLAY);

  } else {
    syntaxError("Invalid PLAY parameters");
  }
}

void Compiler::cmd_draw() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t == 1) {
    i = 0;
    action = current_action->actions[i];
    lexeme = action->lexeme;
    if (lexeme) {
      result_subtype = evalExpression(action);

      if (result_subtype == Lexeme::subtype_string) {
        // call draw function
        if (draw_mark)
          addFix(draw_mark->symbol);
        else
          draw_mark = addMark();
        addCall(0x0000);

      } else {
        syntaxError("Invalid DRAW parameter");
        return;
      }
    }

  } else {
    syntaxError("Invalid DRAW parameters");
  }
}

void Compiler::cmd_dim() {
  Lexeme *lexeme, *parm_lexeme;
  ActionNode* action;
  unsigned int i, k, w, tt, t = current_action->actions.size();
  int new_size;

  if (!t) {
    syntaxError("DIM parameters is missing");
  } else {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->type == Lexeme::type_identifier) {
        if (lexeme->subtype == Lexeme::subtype_string)
          lexeme->x_factor = 256;
        else if (lexeme->subtype == Lexeme::subtype_numeric)
          lexeme->x_factor = 2;
        else if (lexeme->subtype == Lexeme::subtype_single_decimal ||
                 lexeme->subtype == Lexeme::subtype_double_decimal)
          lexeme->x_factor = 3;
        else
          lexeme->x_factor = 0;

        tt = action->actions.size();
        if (tt >= 1 && tt <= 2) {
          lexeme->isArray = true;

          if (tt == 1) {
            k = 0;
            w = 1;
          } else {
            k = 1;
            w = 0;
          }

          parm_lexeme = action->actions[k]->lexeme;
          if (parm_lexeme->type == Lexeme::type_literal &&
              parm_lexeme->subtype == Lexeme::subtype_numeric) {
            try {
              lexeme->x_size = stoi(parm_lexeme->value) + 1;
            } catch (exception& e) {
              printf("Warning: error while converting numeric constant %s\n",
                     parm_lexeme->value.c_str());
              lexeme->x_size = 0;
            }
            if (!lexeme->x_size) {
              syntaxError("Array 1st dimension index cannot be zero");
              break;
            }
          } else {
            syntaxError("Array 1st dimension index must be a integer constant");
            break;
          }

          if (tt == 2) {
            parm_lexeme = action->actions[w]->lexeme;
            if (parm_lexeme->type == Lexeme::type_literal &&
                parm_lexeme->subtype == Lexeme::subtype_numeric) {
              try {
                lexeme->y_size = stoi(parm_lexeme->value) + 1;
              } catch (exception& e) {
                printf("Warning: error while converting numeric constant %s\n",
                       parm_lexeme->value.c_str());
                lexeme->y_size = 0;
              }
              if (!lexeme->y_size) {
                syntaxError("Array 2nd dimension index cannot be zero");
                break;
              }
            } else {
              syntaxError(
                  "Array 2nd dimension index must be a integer constant");
              break;
            }
          } else
            lexeme->y_size = 1;

          lexeme->y_factor = lexeme->x_factor * lexeme->x_size;
          new_size = lexeme->y_factor * lexeme->y_size;

          if (lexeme->array_size < new_size) lexeme->array_size = new_size;
        } else {
          syntaxError("Arrays with more than 2 dimensions isn't supported");
          break;
        }

      } else {
        syntaxError("Invalid DIM parameter");
        break;
      }
    }
  }
}

void Compiler::cmd_redim() {
  cmd_dim();
}

void Compiler::cmd_let() {
  Lexeme* lexeme;
  ActionNode *action, *lex_action;
  unsigned int t = current_action->actions.size();
  int result_subtype, result[3];

  if (t != 2) {
    syntaxError("Invalid LET parameters count");
    return;
  }

  lex_action = current_action->actions[0];
  lexeme = lex_action->lexeme;

  // ld hl, data parameter

  action = current_action->actions[1];
  result_subtype = evalExpression(action);

  // do assignment

  if (lexeme->value == "MID$") {
    // cast
    addCast(result_subtype, Lexeme::subtype_string);

    // push hl
    addPushHL();

    t = lex_action->actions.size();

    if (t < 2 || t > 3) {
      syntaxError("Invalid MID$ assignment parameters count");
      return;
    }

    if (!evalOperatorParms(lex_action, t)) {
      syntaxError("Invalid MID$ assignment parameters");
      return;
    }

    result[0] = lex_action->actions[0]->subtype;
    result[1] = lex_action->actions[1]->subtype;

    if (t == 2) {
      if (result[0] == Lexeme::subtype_single_decimal ||
          result[0] == Lexeme::subtype_double_decimal) {
        // cast
        addCast(result[0], Lexeme::subtype_numeric);
        result[0] = Lexeme::subtype_numeric;
      }

      if (result[1] == Lexeme::subtype_string &&
          result[0] == Lexeme::subtype_numeric) {
        // ld a, l         ; start char
        addLdAL();
        // pop de          ; de=destination string
        addPopDE();
        // pop hl          ; hl=source string
        addPopHL();

        // ld b, (hl)      ; number of chars (all from source)
        addLdBiHL();

        // call 0x7dd8    ; mid assignment (in: hl=source string, b=size,
        // a=start, de=destination string)
        addCall(def_XBASIC_MID_ASSIGN);

        return;
      }

    } else {
      result[2] = lex_action->actions[2]->subtype;

      if (result[0] == Lexeme::subtype_single_decimal ||
          result[0] == Lexeme::subtype_double_decimal) {
        // cast
        addCast(result[0], Lexeme::subtype_numeric);
        result[0] = Lexeme::subtype_numeric;
      }

      // ld b, l             ; number of chars
      addLdBL();

      if (result[1] == Lexeme::subtype_single_decimal ||
          result[1] == Lexeme::subtype_double_decimal) {
        // ld a, b
        addLdAB();
        // pop bc
        addPopBC();
        // pop hl
        addPopHL();
        // push af
        addPushAF();
        // cast
        addCast(result[1], Lexeme::subtype_numeric);
        // pop bc
        addPopBC();
        result[1] = Lexeme::subtype_numeric;
      } else {
        // pop hl
        addPopHL();
      }

      // ld a, l         ; start char
      addLdAL();

      if (result[2] == Lexeme::subtype_string &&
          result[1] == Lexeme::subtype_numeric &&
          result[0] == Lexeme::subtype_numeric) {
        // pop de          ; de=destination string
        addPopDE();
        // pop hl          ; hl=source string
        addPopHL();

        // call 0x7dd8    ; mid assignment (in: hl=source string, b=size,
        // a=start, de=destination string)
        addCall(def_XBASIC_MID_ASSIGN);

        return;
      }
    }

    syntaxError("Invalid MID$ assignment type");

  } else if (lexeme->value == "VDP") {
    // cast
    addCast(result_subtype, Lexeme::subtype_numeric);

    // push hl          ; data
    addPushHL();

    t = lex_action->actions.size();

    if (t != 1) {
      syntaxError("Invalid VDP assignment parameters count");
      return;
    }

    if (!evalOperatorParms(lex_action, t)) {
      syntaxError("Invalid VDP assignment parameters");
      return;
    }

    result[0] = lex_action->actions[0]->subtype;

    // cast
    addCast(result[0], Lexeme::subtype_numeric);
    result[0] = Lexeme::subtype_numeric;

    // pop bc           ; data
    addPopBC();

    // call VDP.set     ; VDP assignment (in: bc=data, hl=register)
    addCall(def_vdp_set);

  } else if (lexeme->value == "SPRITE$") {
    // cast
    addCast(result_subtype, Lexeme::subtype_string);

    // push hl
    addPushHL();

    t = lex_action->actions.size();

    if (t != 1) {
      syntaxError("Invalid SPRITE$ assignment parameters count");
      return;
    }

    if (!evalOperatorParms(lex_action, t)) {
      syntaxError("Invalid SPRITE$ assignment parameters");
      return;
    }

    result[0] = lex_action->actions[0]->subtype;

    // cast
    addCast(result[0], Lexeme::subtype_numeric);
    result[0] = Lexeme::subtype_numeric;

    // ld a, l
    addLdAL();
    // pop hl
    addPopHL();

    // call 0x7143     ; xbasic SPRITE assignment (in: a=sprite pattern,
    // hl=string)
    addCall(def_XBASIC_SPRITE_ASSIGN);

  } else {
    // cast

    addCast(result_subtype, lexeme->subtype);

    // do assignment

    addAssignment(lex_action);
  }
}

bool Compiler::addAssignment(ActionNode* action) {
  if (action->lexeme->type == Lexeme::type_keyword) {
    if (action->lexeme->value == "TIME") {
      // ld (0xFC9E), hl    ; JIFFY
      addLdiiHL(0xFC9E);

    } else if (action->lexeme->value == "MAXFILES") {
      // ld a, l
      addLdAL();
      // ld ix, MAXFILES
      addLdIX(def_MAXFILES);
      // call CALBAS
      addCall(def_CALBAS);
      // ei
      addEI();

    } else {
      syntaxError("Invalid KEYWORD/FUNCTION assignment");
    }

  } else if (action->lexeme->type == Lexeme::type_identifier) {
    if (action->lexeme->isArray ||
        action->lexeme->subtype == Lexeme::subtype_string) {
      // push hl
      addPushHL();

      if (action->lexeme->subtype == Lexeme::subtype_single_decimal ||
          action->lexeme->subtype == Lexeme::subtype_double_decimal) {
        // push bc
        addPushBC();
      }

      if (!addVarAddress(action)) return false;

      if (action->lexeme->subtype == Lexeme::subtype_string) {
        // pop de
        addPopDE();
        // ex de,hl
        addExDEHL();

        // call 0x7e9d   ; xbasic copy string (in: hl=source, de=dest; out: hl
        // end of string)
        addCall(def_XBASIC_COPY_STRING);

      } else if (action->lexeme->subtype == Lexeme::subtype_numeric) {
        // pop de
        addPopDE();
        // ld (hl),e
        addLdiHLE();
        // inc hl
        addIncHL();
        // ld (hl),d
        addLdiHLD();

      } else if (action->lexeme->subtype == Lexeme::subtype_single_decimal ||
                 action->lexeme->subtype == Lexeme::subtype_double_decimal) {
        // pop bc
        addPopBC();
        // pop de
        addPopDE();
        // ld (hl),b
        addLdiHLB();
        // inc hl
        addIncHL();
        // ld (hl),e
        addLdiHLE();
        // inc hl
        addIncHL();
        // ld (hl),d
        addLdiHLD();

      } else {
        syntaxError("Invalid assignment");
        return false;
      }

    } else {
      // assignment optimization

      if (action->lexeme->subtype == Lexeme::subtype_numeric) {
        // ld (var), hl
        addFix(action->lexeme);
        addLdiiHL(0x0000);

      } else if (action->lexeme->subtype == Lexeme::subtype_single_decimal ||
                 action->lexeme->subtype == Lexeme::subtype_double_decimal) {
        // ld a, b
        addLdAB();
        // ld (var), a
        addFix(action->lexeme);
        addLdiiA(0x0000);
        // ld (var+1), hl
        addFix(action->lexeme)->step = 1;
        addLdiiHL(0x0000);

      } else {
        syntaxError("Invalid assignment");
        return false;
      }
    }

  } else {
    syntaxError("Invalid constant/expression assignment");
    return false;
  }

  return true;
}

void Compiler::cmd_if() {
  Lexeme *lexeme, *last_lexeme;
  ActionNode *action, *saved_action = current_action, *last_action;
  unsigned int i, t = saved_action->actions.size(), tt;
  int result_subtype;
  FixNode *mark_else = 0, *mark_endif = 0;
  bool isLastActionGoto = false, isElseLikeEndif = true;

  if (!t) {
    syntaxError("IF parameters is missing");
  } else {
    for (i = 0; i < t; i++) {
      action = saved_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->type == Lexeme::type_keyword) {
        if (lexeme->value == "COND") {
          // ld hl, data parameter

          result_subtype = evalExpression(action->actions[0]);

          if (result_subtype == Lexeme::subtype_numeric) {
            // ld a, l
            addLdAL();
            // or h
            addOrH();
            // jp z, ELSE or ENDIF
            mark_else = addMark();
            addJpZ(0x0000);

          } else {
            syntaxError("Invalid condition expression");
            break;
          }

        } else if (lexeme->value == "THEN") {
          tt = action->actions.size();
          if (tt) {
            last_action = action->actions[tt - 1];
            last_lexeme = last_action->lexeme;
            isLastActionGoto = (last_lexeme->type == Lexeme::type_keyword &&
                                last_lexeme->value == "GOTO");
          }

          if (!evalActions(action)) break;

        } else if (lexeme->value == "GOTO") {
          isLastActionGoto = true;

          if (!evalAction(action)) break;

        } else if (lexeme->value == "GOSUB") {
          if (!evalAction(action)) break;

        } else if (lexeme->value == "ELSE") {
          if (!isLastActionGoto) {
            // jp ENDIF
            mark_endif = addMark();
            addJp(0x0000);
          }

          isLastActionGoto = false;
          isElseLikeEndif = false;

          tt = action->actions.size();
          if (tt == 1) {
            last_action = action->actions[0];
            last_lexeme = last_action->lexeme;
            isLastActionGoto = (last_lexeme->type == Lexeme::type_keyword &&
                                last_lexeme->value == "GOTO");
          }

          if (isLastActionGoto) {
            if (last_action->actions.size()) {
              last_lexeme = last_action->actions[0]->lexeme;
            }

            if (last_lexeme->type == Lexeme::type_literal &&
                last_lexeme->subtype == Lexeme::subtype_numeric) {
              // mark ELSE position
              if (mark_else) {
                mark_else->symbol = addSymbol(last_lexeme->value);
              } else {
                syntaxError("ELSE parameter is missing");
              }
            } else {
              syntaxError("Invalid GOTO parameter");
            }

          } else {
            // mark ELSE position
            if (mark_else) mark_else->symbol->address = code_pointer;

            if (!evalActions(action)) break;
          }

        } else {
          syntaxError("Invalid IF syntax");
          break;
        }

      } else {
        syntaxError("Invalid IF parameter type");
        break;
      }
    }

    // mark ENDIF position
    if (mark_endif)
      mark_endif->symbol->address = code_pointer;
    else if (mark_else)
      if (isElseLikeEndif) mark_else->symbol->address = code_pointer;
  }
}

void Compiler::cmd_for() {
  Lexeme *lexeme, *lex_var = 0;
  ActionNode *action, *var_action, *saved_action = current_action;
  unsigned int i, t = saved_action->actions.size();
  int result_subtype;
  ForNextNode* forNext;
  bool has_let = false, has_to = false, has_step = false;

  if (!t) {
    syntaxError("FOR parameters is missing");
  } else {
    for_count++;

    forNext = new ForNextNode();
    forNextStack.push(forNext);

    forNext->index = for_count;
    forNext->tag = current_tag;
    forNext->for_to =
        new Lexeme(Lexeme::type_identifier, Lexeme::subtype_numeric,
                   "FOR_TO_" + to_string(for_count));
    forNext->for_to_action = new ActionNode();
    forNext->for_to_action->lexeme = forNext->for_to;
    addSymbol(forNext->for_to);

    forNext->for_step =
        new Lexeme(Lexeme::type_identifier, Lexeme::subtype_numeric,
                   "FOR_STEP_" + to_string(for_count));
    forNext->for_step_action = new ActionNode();
    forNext->for_step_action->lexeme = forNext->for_step;
    addSymbol(forNext->for_step);

    for (i = 0; i < t; i++) {
      action = saved_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->type == Lexeme::type_keyword) {
        if (lexeme->value == "LET") {
          if (action->actions.size()) {
            var_action = action->actions[0];
            lex_var = var_action->lexeme;
            forNext->for_var = lex_var;
            forNext->for_to->subtype = lex_var->subtype;
            forNext->for_step->subtype = lex_var->subtype;
          } else {
            syntaxError("Invalid FOR expression (variable assignment)");
            return;
          }

          if (lex_var->type != Lexeme::type_identifier) {
            syntaxError("Invalid FOR expression (variable is missing)");
            return;
          } else {
            if (lex_var->subtype != Lexeme::subtype_numeric &&
                lex_var->subtype != Lexeme::subtype_single_decimal &&
                lex_var->subtype != Lexeme::subtype_double_decimal) {
              syntaxError("Invalid FOR expression (wrong data type)");
              return;
            }
          }

          if (!evalAction(action)) return;

          has_let = true;

        } else if (lexeme->value == "TO") {
          // ld hl, data parameter

          result_subtype = evalExpression(action->actions[0]);

          if (result_subtype == Lexeme::subtype_numeric ||
              result_subtype == Lexeme::subtype_single_decimal ||
              result_subtype == Lexeme::subtype_double_decimal) {
            addCast(result_subtype, lex_var->subtype);

            addAssignment(forNext->for_to_action);

            has_to = true;

          } else {
            syntaxError("Invalid TO expression (wrong data type)");
            return;
          }

        } else if (lexeme->value == "STEP") {
          // ld hl, data parameter

          result_subtype = evalExpression(action->actions[0]);

          if (result_subtype == Lexeme::subtype_numeric ||
              result_subtype == Lexeme::subtype_single_decimal ||
              result_subtype == Lexeme::subtype_double_decimal) {
            addCast(result_subtype, lex_var->subtype);

            addAssignment(forNext->for_step_action);

            has_step = true;

          } else {
            syntaxError("Invalid STEP expression (wrong data type)");
            return;
          }

        } else {
          syntaxError("Invalid FOR syntax");
          return;
        }

      } else {
        syntaxError("Invalid FOR parameter type");
        return;
      }
    }

    if (has_let && has_to) {
      if (!has_step && lex_var->subtype != Lexeme::subtype_numeric) {
        // ld hl, 1
        addLdHL(0x0001);

        addCast(Lexeme::subtype_numeric, forNext->for_step->subtype);

        addAssignment(forNext->for_step_action);
      }

      if (lex_var->subtype == Lexeme::subtype_numeric) {
        // ld hl, (variable)
        addFix(forNext->for_var);
        addLdHLii(0x0000);

        // jr $+12      ; jump to check code
        if (has_step) {
          addJr(0x0B);
        } else {
          addJr(0x07);
        }

        // ;step code
        forNext->for_step_mark = addPreMark();
        forNext->for_step_mark->address = code_pointer;

        // ld hl, (variable)
        addFix(forNext->for_var);
        addLdHLii(0x0000);

        if (has_step) {
          // ld de, (step)
          addByte(0xED);
          addFix(forNext->for_step);
          addCmd(0x5B, 0x0000);
          // add hl,de
          addAddHLDE();
        } else {
          // inc hl
          addIncHL();
        }

        // ld (variable), hl
        addFix(forNext->for_var);
        addLdiiHL(0x0000);

        // ;check code

        // ex de, hl         ; after, de = (variable)
        addExDEHL();

        // ld hl, (to)
        addFix(forNext->for_to);
        addLdHLii(0x0000);

        if (has_step) {
          // ld a, (step+1)
          addFix(forNext->for_step)->step = 1;
          addLdAii(0x0000);

          // bit 7, a
          addWord(0xCB, 0x7F);
          // jr z, $+2
          addJrZ(0x01);
          //   ex de,hl
          addExDEHL();
        }

        // ;var > to? goto end for

        // call intCompareGT
        // addCall(def_intCompareGT);
        addKernelCall(def_intCompareGT);

        // jp nz, end_for
        forNext->for_end_mark = addMark();
        addJpNZ(0x0000);

        // body start

      } else {
        // jr $+26      ; jump to check code
        addJr(0x19);

        // ;step code
        forNext->for_step_mark = addPreMark();
        forNext->for_step_mark->address = code_pointer;

        // ld a, (variable)
        addFix(forNext->for_var);
        addLdAii(0x0000);
        // ld b, a
        addLdBA();
        // ld hl, (variable+1)
        addFix(forNext->for_var)->step = 1;
        addLdHLii(0x0000);

        // ld a, (step)
        addFix(forNext->for_step);
        addLdAii(0x0000);
        // ld c, a
        addLdCA();
        // ld de, (step)
        addByte(0xED);
        addFix(forNext->for_step)->step = 1;
        addCmd(0x5B, 0x0000);
        // call 0x76c1     ; add floats (b:hl + c:de = b:hl)
        addCall(def_XBASIC_ADD_FLOATS);

        // ld a, b
        addLdAB();
        // ld (variable), a
        addFix(forNext->for_var);
        addLdiiA(0x0000);
        // ld (variable+1), hl
        addFix(forNext->for_var)->step = 1;
        addLdiiHL(0x0000);

        // ;check code

        // ld a, (to)
        addFix(forNext->for_to);
        addLdAii(0x0000);
        // ld b, a
        addLdBA();
        // ld hl, (to+1)
        addFix(forNext->for_to)->step = 1;
        addLdHLii(0x0000);

        // ld a, (variable)
        addFix(forNext->for_var);
        addLdAii(0x0000);
        // ld c, a
        addLdCA();
        // ld de, (variable+1)
        addByte(0xED);
        addFix(forNext->for_var)->step = 1;
        addCmd(0x5B, 0x0000);

        // ld a, (step+2)
        addFix(forNext->for_step)->step = 2;
        addLdAii(0x0000);

        // bit 7, a
        addWord(0xCB, 0x7F);
        // jr nz, $+5
        addJrNZ(0x04);
        //   ex de,hl
        addExDEHL();
        //   ld a, c
        addLdAC();
        //   ld c, b
        addLdCB();
        //   ld b, a
        addLdBA();

        // ;var > to? goto end for

        // 78a4 xbasic compare floats (<=)
        addCall(def_XBASIC_COMPARE_FLOATS_LE);

        // ld a, l
        addLdAL();
        // or h
        addOrH();

        // jp z, end_for
        forNext->for_end_mark = addMark();
        addJpZ(0x0000);

        // body start
      }

    } else {
      syntaxError("Incomplete FOR syntax");
    }
  }
}

void Compiler::cmd_next() {
  ForNextNode* forNext;

  if (forNextStack.size()) {
    forNext = forNextStack.top();
    forNextStack.pop();

    // jp step
    addFix(forNext->for_step_mark);
    addJp(0x0000);

    if (forNext->for_end_mark)
      forNext->for_end_mark->symbol->address = code_pointer;

  } else {
    syntaxError("NEXT without a FOR");
  }
}

void Compiler::cmd_locate() {
  ActionNode* action;
  unsigned int t = current_action->actions.size();
  int result_subtype;

  if (t != 2) {
    syntaxError("LOCATE without enough parameters");
  } else {
    action = current_action->actions[0];
    result_subtype = evalExpression(action);

    addCast(result_subtype, Lexeme::subtype_numeric);

    // push hl
    addPushHL();

    action = current_action->actions[1];
    result_subtype = evalExpression(action);

    addCast(result_subtype, Lexeme::subtype_numeric);

    // pop de
    addByteOptimized(0xD1);
    // call XBASIC_LOCATE    ; hl = y, de = x
    addCall(def_XBASIC_LOCATE);
  }
}

void Compiler::cmd_screen() {
  ActionNode* action;
  Lexeme* lexeme;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t) {
    action = current_action->actions[0];
    lexeme = action->lexeme;
    if (lexeme->value == "COPY") {
      current_action = action;
      cmd_screen_copy();
      return;
    } else if (lexeme->value == "PASTE") {
      current_action = action;
      cmd_screen_paste();
      return;
    } else if (lexeme->value == "SCROLL") {
      current_action = action;
      cmd_screen_scroll();
      return;
    } else if (lexeme->value == "LOAD") {
      current_action = action;
      cmd_screen_load();
      return;
    } else if (lexeme->value == "ON") {
      current_action = action;
      cmd_screen_on();
      return;
    } else if (lexeme->value == "OFF") {
      current_action = action;
      cmd_screen_off();
      return;
    }

    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      result_subtype = evalExpression(action);

      if (result_subtype == Lexeme::subtype_null) continue;

      addCast(result_subtype, Lexeme::subtype_numeric);

      if (i != 5) {
        // ld a, l
        addLdAL();
      }

      switch (i) {
        // display mode
        case 0: {
          // call XBASIC_SCREEN ; xbasic SCREEN change mode (in: a, l = screen
          // mode)
          addCall(def_XBASIC_SCREEN);
        } break;

        // sprite size
        case 1: {
          // call 0x70bc    ; xbasic SCREEN sprite (in: a = sprite mode)
          addCall(def_XBASIC_SCREEN_SPRITE);
        } break;

        // key click
        case 2: {
          // ld (CLIKSW), a   ; 0=keyboard click off, 1=keyboard click on
          addLdiiA(def_CLIKSW);
        } break;

        // baud rate
        case 3: {
          // ; original code: C1F63 on subrom of TurboR and A7A2D on main rom of
          // the rest ld bc,5
          addLdBC(0x0005);
          // and a
          addAndA();
          // ld hl,CS1200
          addLdHL(def_CS1200);
          // jr z,skip
          addJrZ(0x01);
          //   add hl,bc
          addAddHLBC();
          // skip:
          // ld de,LOW
          addLdDE(def_LOW);
          // ldir
          addLDIR();
        } break;

        // printer type
        case 4: {
          // ld (NTMSXP), a   ; printer type (0=default)
          addLdiiA(def_NTMSXP);
        } break;

        // interlace mode
        case 5: {
          // ld a, (VERSION)
          addLdAii(def_VERSION);
          // and a
          addAndA();
          // jr z, skip1
          addJrZ(25);

          //   ld a, l
          addLdAL();

          //   ; original code: J1F45 on subrom of MSX2 and above
          //   and 3     ; must be 0 to 3
          addAnd(0x03);
          //   add a, a
          addAddA();
          //   bit 1, a
          addWord(0xCB, 0x4F);
          //   jr z, skip2
          addJrZ(0x02);
          //     set 3, a
          addWord(0xCB, 0xDF);
          //   skip2:
          //   and 0x0C
          addAnd(0x0C);
          //   ld b, a
          addLdBA();
          //   ld a, (RG9SAV)
          addLdAii(def_RG9SAV);
          //   and 0xF3
          addAnd(0xF3);
          //   or b
          addOrB();
          //   ld c, 9
          addLdC(0x09);
          //   ld b, a
          addLdBA();
          //   call WRTVDP
          addCall(def_WRTVDP);

          // skip1:
        } break;

        default: {
          syntaxError("SCREEN parameters not supported");
          return;
        }
      }
    }

  } else {
    syntaxError("SCREEN with empty parameters");
  }
}

void Compiler::cmd_screen_copy() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t) {
    if (t > 2) {
      syntaxError("SCREEN COPY with excess of parameters");
      return;
    }

    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;

      if (i) {
        // push hl
        addPushHL();

        result_subtype = evalExpression(action);
        addCast(result_subtype, Lexeme::subtype_numeric);

        // ld a, l                 ; copy parameter to A
        addLdAL();

        // pop hl
        addPopHL();
      } else {
        if (lexeme->type == Lexeme::type_identifier) {
          // ld hl, variable
          addFix(lexeme);
          addLdHL(0x0000);
          result_subtype = Lexeme::subtype_numeric;
        } else {
          result_subtype = evalExpression(action);
        }
        addCast(result_subtype, Lexeme::subtype_numeric);
        if (t == 1) {
          // xor a
          addXorA();
        }
      }
    }

    // call screen_copy
    addCall(def_cmd_screen_copy);

  } else {
    syntaxError("SCREEN COPY with empty parameters");
  }
}

void Compiler::cmd_screen_paste() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t) {
    if (t > 1) {
      syntaxError("SCREEN PASTE with excess of parameters");
      return;
    }

    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->type == Lexeme::type_identifier) {
        // ld hl, variable
        addFix(lexeme);
        addLdHL(0x0000);
        result_subtype = Lexeme::subtype_numeric;
      } else {
        result_subtype = evalExpression(action);
      }
      addCast(result_subtype, Lexeme::subtype_numeric);
    }

    // call screen_paste
    addCall(def_cmd_screen_paste);

  } else {
    syntaxError("SCREEN PASTE with empty parameters");
  }
}

void Compiler::cmd_screen_scroll() {
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t) {
    if (t > 1) {
      syntaxError("SCREEN SCROLL with excess of parameters");
      return;
    }

    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      result_subtype = evalExpression(action);
      addCast(result_subtype, Lexeme::subtype_numeric);

      // ld a, l                 ; copy parameter to A
      addLdAL();

      // ld hl, (HEAPSTR)
      addLdHLii(def_HEAPSTR);

      // push hl
      addPushHL();

      // call screen_copy
      addCall(def_cmd_screen_copy);

      // pop hl
      addPopHL();

      // call screen_paste
      addCall(def_cmd_screen_paste);
    }

  } else {
    syntaxError("SCREEN SCROLL with empty parameters");
  }
}

void Compiler::cmd_screen_load() {
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      result_subtype = evalExpression(action);
      addCast(result_subtype, Lexeme::subtype_numeric);

      // ld (DAC), hl
      addLdiiHL(def_DAC);

      // call screen_load
      addCall(def_cmd_screen_load);
    }

  } else {
    syntaxError("SCREEN LOAD with empty parameters");
  }
}

void Compiler::cmd_screen_on() {
  // call cmd_enascr
  addCall(def_cmd_enascr);
}

void Compiler::cmd_screen_off() {
  // call cmd_disscr
  addCall(def_cmd_disscr);
}

void Compiler::cmd_width() {
  ActionNode* action;
  unsigned int t = current_action->actions.size();
  int result_subtype;

  if (t == 1) {
    action = current_action->actions[0];
    result_subtype = evalExpression(action);

    addCast(result_subtype, Lexeme::subtype_numeric);

    // call XBASIC WIDTH   ; xbasic WIDTH (in: l = size)
    addCall(def_XBASIC_WIDTH);

  } else {
    syntaxError("WIDTH syntax error");
  }
}

void Compiler::cmd_color() {
  ActionNode *action, *subaction;
  Lexeme* lexeme;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t) {
    action = current_action->actions[0];
    lexeme = action->lexeme;

    if (lexeme->type == Lexeme::type_keyword) {
      if (lexeme->value == "NEW") {
        // ld a, (VERSION)
        addLdAii(def_VERSION);
        // and a
        addAndA();
        // jr z, skip
        addJrZ(8);
        //   ld ix, 0x0141
        addLdIX(0x0141);
        //   call EXTROM
        addCall(def_EXTROM);
        //   ei
        addEI();
        // skip:

      } else if (lexeme->value == "RESTORE") {
        // ld a, (VERSION)
        addLdAii(def_VERSION);
        // and a
        addAndA();
        // jr z, skip
        addJrZ(8);
        //   ld ix, 0x0145
        addLdIX(def_RSTPLT);
        //   call EXTROM
        addCall(def_EXTROM);
        //   ei
        addEI();
        // skip:

      } else if (lexeme->value == "SPRITE") {
        t = action->actions.size();

        if (t != 2) {
          syntaxError("Invalid COLOR SPRITE parameters count");
          return;
        }

        for (i = 0; i < t; i++) {
          subaction = action->actions[i];
          result_subtype = evalExpression(subaction);

          if (result_subtype != Lexeme::subtype_null) {
            addCast(result_subtype, Lexeme::subtype_numeric);

            // ld a, l
            addLdAL();
          }

          if (i < 1) {
            // push af
            addPushAF();
          }
        }

        // pop bc
        addPopBC();
        // call COLOR_SPRITE   ; in: b, a
        addCall(def_XBASIC_COLOR_SPRITE);

      } else if (lexeme->value == "SPRITE$") {
        t = action->actions.size();

        if (t != 2) {
          syntaxError("Invalid COLOR SPRITE$ parameters count");
          return;
        }

        for (i = 0; i < t; i++) {
          subaction = action->actions[i];
          result_subtype = evalExpression(subaction);

          if (i < 1) {
            addCast(result_subtype, Lexeme::subtype_numeric);

            // ld a, l
            addLdAL();
            // push af
            addPushAF();
          }
        }

        // pop bc
        addPopBC();
        // call COLOR_SPRSTR   ; in: b, hl
        addCall(def_XBASIC_COLOR_SPRSTR);

      } else if (lexeme->value == "RGB") {
        t = action->actions.size();

        if (t < 2 || t > 4) {
          syntaxError("Invalid COLOR RGB parameters count");
          return;
        }

        for (i = 0; i < t; i++) {
          subaction = action->actions[i];
          result_subtype = evalExpression(subaction);

          if (result_subtype != Lexeme::subtype_null) {
            addCast(result_subtype, Lexeme::subtype_numeric);

            // ld a, l
            addLdAL();
          }

          if (i < 3) {
            // push af
            addPushAF();
          }
        }

        if (t < 4) {
          // xor a
          addXorA();
        }
        // ld b, a
        addLdBA();

        if (t < 3) {
          // ld h, a
          addLdHA();
        } else {
          // pop hl
          addPopHL();
        }

        // pop de
        addPopDE();
        // pop af
        addPopAF();

        // call COLOR_RGB   ; in: a, d, h, b
        addCall(def_XBASIC_COLOR_RGB);

      } else {
        syntaxError("Invalid COLOR parameters");
      }

    } else {
      for (i = 0; i < t; i++) {
        action = current_action->actions[i];
        result_subtype = evalExpression(action);

        if (result_subtype == Lexeme::subtype_null) continue;

        addCast(result_subtype, Lexeme::subtype_numeric);

        // ld a, l
        addLdAL();

        switch (i) {
          case 0: {
            // ld (0xF3E9), a   ; FORCLR
            addLdiiA(0xF3E9);
            // ld (ATRBYT), a   ; ATRBYT
            addLdiiA(def_ATRBYT);
          } break;

          case 1: {
            // ld (0xF3EA), a   ; BAKCLR
            addLdiiA(0xF3EA);
          } break;

          case 2: {
            // ld (0xF3EB), a   ; BDRCLR
            addLdiiA(0xF3EB);
          } break;

          default: {
            syntaxError("COLOR parameters not supported");
            return;
          }
        }
      }

      // ld a, (SCRMOD)
      addLdAii(def_SCRMOD);

      // call 0x0062   ; CHGCLR
      addCall(0x0062);
    }

  } else {
    syntaxError("COLOR with empty parameters");
  }
}

void Compiler::cmd_pset(bool forecolor) {
  ActionNode *action, *sub_action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;
  bool has_x_coord = false, has_y_coord = false, has_color = false,
       has_operator = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];

      switch (i) {
        case 0: {
          if (action->actions.size() != 2) {
            if (forecolor)
              syntaxError("Coordenates parameters error on PSET");
            else
              syntaxError("Coordenates parameters error on PRESET");
            return;
          } else if (action->lexeme->value == "COORD") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);

              // push hl
              addPushHL();

              has_y_coord = true;
            }

          } else if (action->lexeme->value == "STEP") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB7)  ;GRPACX
              addLdDEii(0xFCB7);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB9)  ;GRPACY
              addLdDEii(0xFCB9);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();

              has_y_coord = true;
            }

          } else {
            if (forecolor)
              syntaxError("Invalid coordenates on PSET");
            else
              syntaxError("Invalid coordenates on PRESET");
            return;
          }

        } break;

        case 1: {
          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();

          // push af       ; save color
          addPushAF();

          has_color = true;

        } break;

        case 2: {
          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();

          // ld (LOGOPR), a          ; save new logical operator to basic
          // interpreter
          addLdiiA(def_LOGOPR);

          has_operator = true;

        } break;

        default: {
          if (forecolor)
            syntaxError("PSET parameters not supported");
          else
            syntaxError("PRESET parameters not supported");
          return;
        }
      }
    }

    if (!has_operator) {
      // ld a, (LOGOPR)      ; get default operator from basic interpreter
      addLdAii(def_LOGOPR);
    }

    // ld b, a      ; get operator
    addLdBA();

    if (has_color) {
      // pop af       ; color
      addPopAF();
    } else {
      if (forecolor) {
        // ld a, (0xF3E9)       ; FORCLR
        addLdAii(def_FORCLR);
      } else {
        // ld a, (0xF3EA)       ; BAKCLR
        addLdAii(def_BAKCLR);
      }
    }

    if (has_y_coord) {
      // pop hl
      addPopHL();
    } else {
      // ld hl, (0xFCB9)  ;GRPACY
      addLdHLii(0xFCB9);
    }

    if (has_x_coord) {
      // pop de
      addPopDE();
    } else {
      // ld de, (0xFCB7)  ;GRPACX
      addLdDEii(0xFCB7);
    }

    // call 0x6F71   ; xbasic PSET (in: hl=y, de=x, a=color, b=operator)
    addCall(def_XBASIC_PSET);

  } else {
    if (forecolor)
      syntaxError("PSET with empty parameters");
    else
      syntaxError("PRESET with empty parameters");
  }
}

void Compiler::cmd_paint() {
  ActionNode *action, *sub_action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;
  bool has_x_coord = false, has_y_coord = false, has_color = false,
       has_border = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];

      switch (i) {
        case 0: {
          if (action->actions.size() != 2) {
            syntaxError("Coordenates parameters error on PAINT");
            return;
          } else if (action->lexeme->value == "COORD") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);

              // push hl
              addPushHL();

              has_y_coord = true;
            }

          } else if (action->lexeme->value == "STEP") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB7)  ;GRPACX
              addLdDEii(0xFCB7);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);
              // push hl
              addPushHL();

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB9)  ;GRPACY
              addLdDEii(0xFCB9);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);
              // push hl
              addPushHL();

              has_y_coord = true;
            }

          } else {
            syntaxError("Invalid coordenates on PAINT");
            return;
          }

        } break;

        case 1: {
          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld b, l       ; paint color
          addLdBL();

          has_color = true;

        } break;

        case 2: {
          if (has_color) {
            // push bc    ; save paint color
            addPushBC();
          }

          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          has_border = true;

        } break;

        default: {
          syntaxError("PAINT parameters not supported");
          return;
        }
      }
    }

    if (has_border) {
      if (has_color) {
        // pop bc               ; restore paint color
        addPopBC();
      } else {
        // ld a, (ATRBYT)
        addLdAii(def_ATRBYT);
        // ld b, a              ; paint color = default color
        addLdBA();
      }
      //   ld a, l                ; border color
      addLdAL();

    } else {
      if (has_color) {
        // ld a, l              ; border color = paint color
        addLdAL();
      } else {
        // ld a, (ATRBYT)       ; border color = default color
        addLdAii(def_ATRBYT);
        // ld b, a              ; paint color = default color
        addLdBA();
      }
    }

    if (has_y_coord) {
      // pop hl
      addPopHL();
    } else {
      // ld hl, (0xFCB9)  ;GRPACY
      addLdHLii(0xFCB9);
    }

    if (has_x_coord) {
      // pop de
      addPopDE();
    } else {
      // ld de, (0xFCB7)  ;GRPACX
      addLdDEii(0xFCB7);
    }

    // call 0x74B3   ; xbasic PAINT (in: hl=y, de=x, b=filling color, a=border
    // color)
    addCall(def_XBASIC_PAINT);

  } else {
    syntaxError("PAINT with empty parameters");
  }
}

void Compiler::cmd_circle() {
  ActionNode *action, *sub_action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;
  bool has_x_coord = false, has_y_coord = false, has_radius = false,
       has_color = false;
  bool has_trace1 = false, has_trace2 = false, has_aspect = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];

      switch (i) {
        // coord
        case 0: {
          if (action->actions.size() != 2) {
            syntaxError("Coordenates parameters error on CIRCLE");
            return;
          } else if (action->lexeme->value == "COORD") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);

              has_y_coord = true;
            }

          } else if (action->lexeme->value == "STEP") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB7)  ;GRPACX
              addLdDEii(0xFCB7);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB9)  ;GRPACY
              addLdDEii(0xFCB9);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);

              has_y_coord = true;
            }

          } else {
            syntaxError("Invalid coordenates on CIRCLE");
            return;
          }

        } break;

        // radius
        case 1: {
          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          has_radius = true;

        } break;

        // color
        case 2: {
          if (has_radius) {
            // push hl
            addPushHL();
          }

          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) {
            if (has_radius) {
              code_pointer--;
              code_size--;
            }
            continue;
          }

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();

          has_color = true;

        } break;

        // tracing start
        case 3: {
          if (has_radius) {
            if (has_color) {
              // pop hl
              addPopHL();
            }
            // ld c, l
            addLdCL();
          }

          // ld hl, BUF
          addLdHL(def_BUF);
          // push hl
          addPushHL();
          //   ld (hl), 0x2C  ; comma
          // addLdiHL(0x2C);
          //   inc hl
          // addIncHL();

          if (has_radius) {
            //   ld (hl), 0x0F  ; short interger marker
            addLdiHL(0x0F);
            //   inc hl
            addIncHL();
            //   ld (hl), c     ; radius
            addLdiHLC();
            //   inc hl
            addIncHL();
          }

          //   ld (hl), 0x2C  ; comma
          addLdiHL(0x2C);
          //   inc hl
          addIncHL();

          if (has_color) {
            //   ld (hl), 0x0F  ; short interger marker
            addLdiHL(0x0F);
            //   inc hl
            addIncHL();
            //   ld (hl), a     ; color
            addLdiHLA();
            //   inc hl
            addIncHL();
          }

          //   ld (hl), 0x2C  ; comma
          addLdiHL(0x2C);
          //   inc hl
          addIncHL();

          // push hl
          addPushHL();

          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) {
            code_pointer--;
            code_size--;
            continue;
          }

          addCast(result_subtype, Lexeme::subtype_single_decimal);

          // pop de
          addByteOptimized(0xD1);

          // call WriteParamBCD             ; b:hl, de -> hl
          addCall(def_WriteParamBCD);

          has_trace1 = true;

        } break;

        // tracing end
        case 4: {
          //   ld (hl), 0x2C  ; comma
          addLdiHL(0x2C);
          //   inc hl
          addIncHL();

          // push hl
          addPushHL();

          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) {
            code_pointer--;
            code_size--;
            continue;
          }

          addCast(result_subtype, Lexeme::subtype_single_decimal);

          // pop de
          addByteOptimized(0xD1);

          // call WriteParamBCD             ; b:hl, de -> hl
          addCall(def_WriteParamBCD);

          has_trace2 = true;
        } break;

        // aspect ratio
        case 5: {
          //   ld (hl), 0x2C  ; comma
          addLdiHL(0x2C);
          //   inc hl
          addIncHL();

          // push hl
          addPushHL();

          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) {
            code_pointer -= 3;
            code_size -= 3;
            continue;
          }

          addCast(result_subtype, Lexeme::subtype_single_decimal);

          // pop de
          addByteOptimized(0xD1);

          // call WriteParamBCD             ; b:hl, de -> hl
          addCall(def_WriteParamBCD);

          has_aspect = true;
        } break;

        default: {
          syntaxError("Invalid CIRCLE parameters");
          return;
        }
      }
    }

    if (has_trace1 || has_trace2 || has_aspect) {
      //   ld (hl), 0x00
      addLdiHL(0x00);
      //   inc hl
      addIncHL();

      // pop hl
      addPopHL();

      // ld a, (hl)         ; first character
      addLdAiHL();
      // ld ix, M5B16       ; rom basic circle without coords
      addLdIX(0x5B16);
      // call xbasic CIRCLE2 (in: hl = basic line starting on radius parameter)
      addCall(def_XBASIC_CIRCLE2);

    } else {
      if (has_color && has_radius) {
        // pop hl
        addPopHL();
      } else if (!has_color) {
        // ld a, (0xF3E9)       ; FORCLR
        addLdAii(0xF3E9);
      }

      if (!has_radius) {
        // ld hl, 0
        addLdHL(0x0000);
      }

      if (!has_y_coord) {
      }

      if (!has_x_coord) {
      }

      // call xbasic CIRCLE (in: GRPACX/GRPACY, hl=radius, a=color)
      addCall(def_XBASIC_CIRCLE);
    }

  } else {
    syntaxError("CIRCLE with empty parameters");
  }
}

void Compiler::cmd_put() {
  ActionNode* action;
  Lexeme* lexeme;
  unsigned int t = current_action->actions.size();

  if (t) {
    action = current_action->actions[0];
    lexeme = action->lexeme;

    if (lexeme->type == Lexeme::type_keyword && lexeme->value == "SPRITE") {
      current_action = action;
      cmd_put_sprite();
    } else if (lexeme->type == Lexeme::type_keyword &&
               lexeme->value == "TILE") {
      current_action = action;
      cmd_put_tile();
    } else {
      syntaxError("Invalid PUT statement");
    }

  } else {
    syntaxError("Empty PUT statement");
  }
}

void Compiler::cmd_put_sprite() {
  ActionNode *action, *sub_action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype, parm_flag = 0;
  bool has_sprite = false, has_x_coord = false, has_y_coord = false,
       has_color = false, has_pattern = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];

      switch (i) {
        case 0: {
          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();
          // push af
          addPushAF();

          has_sprite = true;

        } break;

        case 1: {
          if (action->actions.size() != 2) {
            syntaxError("Coordenates parameters error on PUT SPRITE");
            return;
          } else if (action->lexeme->value == "COORD") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);

              // push hl
              addPushHL();

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);

              // push hl
              addPushHL();

              has_y_coord = true;
            }

          } else if (action->lexeme->value == "STEP") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB7)  ;GRPACX
              addLdDEii(0xFCB7);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);

              // push hl
              addPushHL();

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB9)  ;GRPACY
              addLdDEii(0xFCB9);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);

              // push hl
              addPushHL();

              has_y_coord = true;
            }

          } else {
            syntaxError("Invalid coordenates on PUT SPRITE");
            return;
          }

        } break;

        case 2: {
          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();
          // push af
          addPushAF();

          has_color = true;

        } break;

        case 3: {
          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();
          // push af
          addPushAF();

          has_pattern = true;

        } break;

        default: {
          syntaxError("PUT SPRITE parameters not supported");
          return;
        }
      }
    }

    if (has_pattern) {
      // pop hl
      addPopHL();
      parm_flag |= 0x20;
    } else {
      // ld h, 0
      // addLdHn(0x00);
    }

    if (has_color) {
      // pop de
      addPopDE();
      parm_flag |= 0x40;
    } else {
      // ld d, 0
      // addWord(0x16, 0x00);
    }

    if (has_y_coord) {
      // pop iy
      addPopIY();
      parm_flag |= 0x80;
    } else {
      // ld iy, 0x0000
      // addLdIYii(0x0000);
    }

    if (has_x_coord) {
      // pop ix
      addPopIX();
      parm_flag |= 0x80;
    } else {
      // ld ix, 0x0000
      // addLdIXnn(0x0000);
    }

    if (has_sprite) {
      // pop af
      addPopAF();
    } else {
      // xor a
      addXorA();
    }

    // ld b, parameters flag
    addLdB(parm_flag);

    // call xbasic PUT SPRITE (in: ix=x, iy=y, d=color, a=sprite number,
    // h=pattern number, b=parameters flag (b11100000)
    addCall(def_XBASIC_PUT_SPRITE);

  } else {
    syntaxError("PUT SPRITE with empty parameters");
  }
}

void Compiler::cmd_put_tile() {
  ActionNode *action, *sub_action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;
  bool has_tile = false, has_x_coord = false, has_y_coord = false;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];

      switch (i) {
        case 0: {
          result_subtype = evalExpression(action);

          if (result_subtype == Lexeme::subtype_null) continue;

          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();
          // push af
          addPushAF();

          has_tile = true;

        } break;

        case 1: {
          if (action->actions.size() != 2) {
            syntaxError("Coordenates parameters error on PUT TILE");
            return;
          } else if (action->lexeme->value == "COORD") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);

              // push hl
              addPushHL();

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);

              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);

              // push hl
              addPushHL();

              has_y_coord = true;
            }

          } else if (action->lexeme->value == "STEP") {
            sub_action = action->actions[0];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB7)  ;GRPACX
              addLdDEii(0xFCB7);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB7), hl  ;GRPACX
              addLdiiHL(0xFCB7);

              // push hl
              addPushHL();

              has_x_coord = true;
            }

            sub_action = action->actions[1];

            result_subtype = evalExpression(sub_action);

            if (result_subtype == Lexeme::subtype_null) {
            } else {
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld de, (0xFCB9)  ;GRPACY
              addLdDEii(0xFCB9);
              // add hl, de
              addAddHLDE();
              // ld (0xFCB9), hl  ;GRPACY
              addLdiiHL(0xFCB9);

              // push hl
              addPushHL();

              has_y_coord = true;
            }

          } else {
            syntaxError("Invalid coordenates on PUT TILE");
            return;
          }

        } break;

        default: {
          syntaxError("PUT TILE parameters not supported");
          return;
        }
      }
    }

    if (has_y_coord) {
      // pop hl
      addPopHL();
      // inc l        ; y coord (1 based for bios)
      addIncL();
    } else {
      // ld a, (CSRY)
      addLdAii(def_CSRY);
      // ld l, a
      addLdLA();
    }

    if (has_x_coord) {
      // pop bc
      addPopBC();
      // ld h, c
      addLdHC();
      // inc h        ; x coord (1 based for bios)
      addIncH();
    } else {
      // ld a, (CSRX)
      addLdAii(def_CSRX);
      // ld h, a
      addLdHA();
    }

    //   call TileAddress   ; in hl=xy, out: hl
    addCall(def_tileAddress);

    if (has_tile) {
      // pop af
      addPopAF();
    } else {
      // xor a
      addXorA();
    }

    // call 0x70b5                  ; xbasic VPOKE (in: hl=address, a=byte)
    addCall(def_XBASIC_VPOKE);

  } else {
    syntaxError("PUT TILE with empty parameters");
  }
}

void Compiler::cmd_set() {
  ActionNode* action;
  Lexeme* next_lexeme;
  unsigned int t = current_action->actions.size();
  FixNode* mark;

  if (t == 1) {
    action = current_action->actions[0];
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
    addLdAii(def_VERSION);
    // and a
    addAndA();
    // jp z, $                ; skip if MSX1
    mark = addMark();
    addJpZ(0x0000);

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
      syntaxError("SET PASSWORD will not be supported for end-user security");
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
      syntaxError("Invalid SET statement");
    }

    mark->symbol->address = code_pointer;

  } else {
    syntaxError("Wrong SET parameters count");
  }
}

void Compiler::beginBasicSetStmt(string name) {
  int i, l = name.size();
  char* s = (char*)name.c_str();

  // ld hl, BUF
  addLdHL(def_BUF);
  // push hl
  addPushHL();

  // ; set subcommand
  for (i = 0; i < l; i++) {
    addBasicChar(s[i]);
  }
}

void Compiler::endBasicSetStmt() {
  //   xor a
  addXorA();
  //   ld (hl), a
  addLdiHLA();
  //   inc hl
  addIncHL();
  //   ld (hl), a
  addLdiHLA();

  // pop hl
  addPopHL();

  // ld a, (VERSION)
  addLdAii(def_VERSION);
  // and a
  addAndA();
  // jr z, skip
  addJrZ(9);

  //   ld a, (hl)      ; first character
  addLdAiHL();
  //   ld ix, (SET)    ; SET
  addLdIXii(def_SET_STMT);
  //   call CALBAS
  addCall(def_CALBAS);
  //   ei
  addEI();

  // skip:
}

void Compiler::addBasicChar(char c) {
  // ld (hl), char
  addLdiHL(c);
  // inc hl
  addIncHL();
}

void Compiler::cmd_set_video() {
  ActionNode *action = current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();
  int result_subtype;

  if (t) {
    if (t > 7) {
      syntaxError("Invalid SET VIDEO parameters");
      return;
    }

    beginBasicSetStmt("VIDEO");

    for (i = 0; i < t; i++) {
      if (i) {
        // comma
        addBasicChar(',');
      }

      // push hl
      addPushHL();

      sub_action = action->actions[i];
      result_subtype = evalExpression(sub_action);

      if (result_subtype == Lexeme::subtype_null) {
        code_pointer--;
        code_size--;
        continue;
      }

      addCast(result_subtype, Lexeme::subtype_numeric);

      //   ld a, l
      addLdAL();
      //   and 3
      addAnd(0x03);
      //   inc a
      addIncA();
      //   or 0x10
      addOr(0x10);

      // pop hl
      addPopHL();

      // ld (hl), a
      addLdiHLA();
      // inc hl
      addIncHL();
    }

    endBasicSetStmt();

  } else {
    syntaxError("SET VIDEO with empty parameters");
  }
}

void Compiler::cmd_set_adjust() {
  ActionNode *action = current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();
  int result_subtype;

  if (t == 2) {
    // get parameters

    for (i = 0; i < t; i++) {
      if (i) {
        // push hl
        addPushHL();
      }
      // ld hl, parameter value
      sub_action = action->actions[i];
      result_subtype = evalExpression(sub_action);
      addCast(result_subtype, Lexeme::subtype_numeric);
    }

    // ex de,hl   ; DE = second parameter
    addExDEHL();
    // pop bc     ; BC = first parameter
    addPopBC();

    // build command string

    beginBasicSetStmt("ADJUST");

    // (
    addBasicChar('(');

    // integer prefix
    addBasicChar(0x1C);
    // ld (hl), c      ; first parameter
    addLdiHLC();
    // inc hl
    addIncHL();
    // ld (hl), b
    addLdiHLB();
    // inc hl
    addIncHL();

    // comma
    addBasicChar(',');

    // integer prefix
    addBasicChar(0x1C);
    // ld (hl), e      ; second parameter
    addLdiHLE();
    // inc hl
    addIncHL();
    // ld (hl), d
    addLdiHLD();
    // inc hl
    addIncHL();

    // )
    addBasicChar(')');

    endBasicSetStmt();

  } else {
    syntaxError("Wrong parameters count on SET ADJUST statement");
  }
}

void Compiler::cmd_set_screen() {
  ActionNode* action = current_action->actions[0];
  unsigned int t = action->actions.size();

  if (t == 0) {
    // build command string

    beginBasicSetStmt("");
    addBasicChar(0xC5);  // token for SCREEN

    endBasicSetStmt();

  } else {
    syntaxError("Wrong parameters count on SET SCREEN statement");
  }
}

void Compiler::cmd_set_beep() {
  ActionNode *action = current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();
  int result_subtype;

  if (t == 2) {
    // get parameters

    for (i = 0; i < t; i++) {
      if (i) {
        // push hl
        addPushHL();
      }
      // ld hl, parameter value
      sub_action = action->actions[i];
      result_subtype = evalExpression(sub_action);
      addCast(result_subtype, Lexeme::subtype_numeric);
    }

    // ex de,hl   ; DE = second parameter
    addExDEHL();
    // pop bc     ; BC = first parameter
    addPopBC();

    // build command string

    beginBasicSetStmt("");
    addBasicChar(0xC0);  // token for BEEP

    // short integer prefix
    addBasicChar(0x0F);
    // ld (hl), c      ; first parameter
    addLdiHLC();
    // inc hl
    addIncHL();

    // comma
    addBasicChar(',');

    // short integer prefix
    addBasicChar(0x0F);
    // ld (hl), e      ; second parameter
    addLdiHLE();
    // inc hl
    addIncHL();

    endBasicSetStmt();

  } else {
    syntaxError("Wrong parameters count on SET BEEP statement");
  }
}

void Compiler::cmd_set_title() {
  ActionNode *action = current_action->actions[0], *sub_action;
  unsigned int t = action->actions.size();
  int result_subtype;

  if (t == 2) {
    // get parameters

    // ld hl, parameter value
    sub_action = action->actions[1];
    result_subtype = evalExpression(sub_action);
    addCast(result_subtype, Lexeme::subtype_numeric);

    // push hl
    addPushHL();

    // ld hl, parameter value
    sub_action = action->actions[0];
    result_subtype = evalExpression(sub_action);
    addCast(result_subtype, Lexeme::subtype_string);

    // ex de,hl   ; DE = first parameter
    addExDEHL();
    // pop bc     ; BC = second parameter
    addPopBC();

    // build command string

    beginBasicSetStmt("TITLE");

    // double quote
    addBasicChar('"');
    // push bc
    addPushBC();
    // ex de,hl
    addExDEHL();
    //   ld c, (hl)  ; string size
    addLdCiHL();
    //   ld b, 0
    addLdB(0x00);
    //   inc hl
    addIncHL();
    //   ldir
    addLDIR();
    // ex de,hl
    addExDEHL();
    // pop bc
    addPopBC();
    // double quote
    addBasicChar('"');

    // comma
    addBasicChar(',');

    // short integer prefix
    addBasicChar(0x0F);
    // ld (hl), c      ; second parameter
    addLdiHLC();
    // inc hl
    addIncHL();

    endBasicSetStmt();

  } else {
    syntaxError("Wrong parameters count on SET TITLE statement");
  }
}

void Compiler::cmd_set_prompt() {
  ActionNode *action = current_action->actions[0], *sub_action;
  unsigned int t = action->actions.size();
  int result_subtype;

  if (t == 1) {
    // get parameters

    // ld hl, parameter value
    sub_action = action->actions[0];
    result_subtype = evalExpression(sub_action);
    addCast(result_subtype, Lexeme::subtype_string);

    // ex de,hl   ; DE = first parameter
    addExDEHL();

    // build command string

    beginBasicSetStmt("PROMPT");

    // double quote
    addBasicChar('"');
    // ex de,hl
    addExDEHL();
    //   ld c, (hl)  ; string size
    addLdCiHL();
    //   ld b, 0
    addLdB(0x00);
    //   inc hl
    addIncHL();
    //   ldir
    addLDIR();
    // ex de,hl
    addExDEHL();
    // double quote
    addBasicChar('"');

    endBasicSetStmt();

  } else {
    syntaxError("Wrong parameters count on SET PROMPT statement");
  }
}

void Compiler::cmd_set_page() {
  ActionNode *action = current_action->actions[0], *sub_action;
  Lexeme* lexeme;
  unsigned int t = action->actions.size();
  int result_subtype;

  if (t >= 1 && t <= 2) {
    // ld hl, parameter value
    sub_action = action->actions[0];
    lexeme = sub_action->lexeme;

    if (!(lexeme->type == Lexeme::type_literal &&
          lexeme->subtype == Lexeme::subtype_null)) {
      result_subtype = evalExpression(sub_action);
      addCast(result_subtype, Lexeme::subtype_numeric);

      // ld a,l
      addLdAL();

      // call SET_PAGE       ; in: a = display page
      addCall(def_XBASIC_SET_PAGE);
    }

    if (t == 2) {
      sub_action = action->actions[1];
      result_subtype = evalExpression(sub_action);
      addCast(result_subtype, Lexeme::subtype_numeric);

      //   ld a,l
      addLdAL();
      //   ld (ACPAGE), a    ; in: a = active page (write and read)
      addLdiiA(def_ACPAGE);
    }

  } else {
    syntaxError("Wrong parameters count on SET PAGE statement");
  }
}

void Compiler::cmd_set_scroll() {
  ActionNode *action = current_action->actions[0], *sub_action;
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
        addLdHL(0xFFFF);
      } else {
        // ld hl, parameter value
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
      }

      if (i > 1) {
        // ld h, l
        addLdHL();
      }

      // push hl
      addPushHL();
    }

    if (t == 4) {
      // pop af
      addPopAF();
    } else {
      // ld a, 0xff
      addLdA(0xFF);
    }

    if (t >= 3) {
      // pop bc
      addPopBC();
    } else {
      // ld b, 0xff
      addLdB(0xFF);
    }

    if (t >= 2) {
      // pop hl
      addPopHL();
    } else {
      // ld hl, 0xffff
      addLdHL(0xFFFF);
    }

    // pop de
    addPopDE();

    // call SET_SCROLL      ; in: de=x, hl=y, b=mask mode, a=page mode
    addCall(def_XBASIC_SET_SCROLL);

  } else {
    syntaxError("Wrong parameters count on SET SCROLL statement");
  }
}

void Compiler::cmd_set_tile() {
  ActionNode *action = current_action->actions[0], *sub_action, *sub_sub_action;
  Lexeme *lexeme, *sub_lexeme;
  unsigned int i, t, tt;
  int result_subtype;

  t = action->actions.size();
  if (t) {
    action = action->actions[0];
    lexeme = action->lexeme;
    t = action->actions.size();

    if (lexeme->value == "ON") {
      // ld a, 2                   ; tiled mode
      addLdA(2);
      // ld (SOMODE), a
      addLdiiA(def_SOMODE);
      // clear screen, set font to default and put cursor on home
      cmd_cls();
      // ld hl, 0
      addLdHL(0x0000);
      // ld (DAC), hl
      addLdiiHL(def_DAC);
      // ld d, h
      addLdDH();
      // ld e, l
      addLdEL();
      // call XBASIC_LOCATE    ; hl = y, de = x
      addCall(def_XBASIC_LOCATE);
      // ld a, 0xff                ; it means all screen banks
      addLdA(0xFF);
      // ld (ARG), a
      addLdiiA(def_ARG);
      // call cmd_setfnt
      addCall(def_cmd_setfnt);

    } else if (lexeme->value == "OFF") {
      // ld a, 1      ; graphical mode
      addLdA(1);
      // ld (SOMODE), a
      addLdiiA(def_SOMODE);

    } else if (lexeme->value == "FLIP") {
      if (t == 2) {
        // tile number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; parameter
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // push hl
        addPushHL();

        // direction
        sub_action = action->actions[1];
        // ld hl, parameter value    ; parameter
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // pop de
        addPopDE();

        addCall(def_set_tile_flip);

      } else {
        syntaxError("Wrong parameters count on SET TILE FLIP statement");
      }

    } else if (lexeme->value == "ROTATE") {
      if (t == 2) {
        // tile number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; parameter
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // push hl
        addPushHL();

        // direction
        sub_action = action->actions[1];
        // ld hl, parameter value    ; parameter
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // pop de
        addPopDE();

        addCall(def_set_tile_rotate);

      } else {
        syntaxError("Wrong parameters count on SET TILE ROTATE statement");
      }

    } else if (lexeme->value == "PATTERN") {
      if (t >= 2) {
        sub_action = action->actions[1];
        lexeme = sub_action->lexeme;
        if (t == 2 && lexeme->value != "ARRAY") {
          // tile number
          sub_action = action->actions[0];
          // ld hl, parameter value    ; tile number
          result_subtype = evalExpression(sub_action);
          addCast(result_subtype, Lexeme::subtype_numeric);
          addLdAL();
          addPushAF();

          // tile buffer pointer (8 bytes)
          sub_action = action->actions[1];
          sub_lexeme = sub_action->lexeme;
          if (sub_lexeme->type == Lexeme::type_identifier) {
            // ld hl, variable
            addFix(sub_lexeme);
            addLdHL(0x0000);
            result_subtype = Lexeme::subtype_numeric;
          } else {
            result_subtype = evalExpression(sub_action);
          }
          addCast(result_subtype, Lexeme::subtype_numeric);
          addPopAF();

          // call set_tile_pattern
          //   a = tile number
          //   hl = pointer to an 8 bytes buffer
          addCall(def_set_tile_pattern);
        } else if (t <= 3) {
          // tile number
          sub_action = action->actions[0];
          // ld hl, parameter value    ; tile number
          result_subtype = evalExpression(sub_action);
          addCast(result_subtype, Lexeme::subtype_numeric);
          // ld (ARG), hl
          addLdiiHL(def_ARG);

          // bank number
          if (t == 3) {
            sub_action = action->actions[2];
            // ld hl, parameter value    ; tile number
            result_subtype = evalExpression(sub_action);
            addCast(result_subtype, Lexeme::subtype_numeric);
            // ld h, l
            addLdHL();
          } else {
            // ld h, 0x03
            addLdH(0x03);
          }
          // ld (ARG2), hl
          addLdiiHL(def_ARG2);

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
                result_subtype = evalExpression(sub_sub_action);
                addCast(result_subtype, Lexeme::subtype_numeric);

                // ld bc, (ARG2)
                addLdBCii(def_ARG2);
                // ld c, l
                addLdCL();
                // ld de, *i*
                addLdDE(i);
                // ld hl, (ARG)
                addLdHLii(def_ARG);

                // call set_tile_pattern ; hl = tile number, de = line number, b
                // = bank number (3=all), c = pattern data
                addCall(def_set_tile_pattern);
              }
            }

          } else {
            syntaxError(
                "Wrong pattern parameter on SET TILE PATTERN statement");
          }
        } else {
          syntaxError("Wrong parameters count on SET TILE PATTERN statement");
        }
      } else {
        syntaxError("Wrong parameters count on SET TILE PATTERN statement");
      }

    } else if (lexeme->value == "COLOR") {
      if (t == 2) {
        // tile number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; tile number
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        addLdAL();
        addPushAF();

        // tile buffer pointer (8 bytes)
        sub_action = action->actions[1];
        sub_lexeme = sub_action->lexeme;
        if (sub_lexeme->type == Lexeme::type_identifier) {
          // ld hl, variable
          addFix(sub_lexeme);
          addLdHL(0x0000);
          result_subtype = Lexeme::subtype_numeric;
        } else {
          result_subtype = evalExpression(sub_action);
        }
        addCast(result_subtype, Lexeme::subtype_numeric);
        addPopAF();

        // call set_tile_color
        //   a = tile number
        //   hl = pointer to an 8 bytes buffer
        addCall(def_set_tile_color);
      } else if (t >= 2 && t <= 4) {
        // tile number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; tile number
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // ld (ARG), hl
        addLdiiHL(def_ARG);

        // bank number
        if (t == 4) {
          sub_action = action->actions[3];
          // ld hl, parameter value    ; tile number
          result_subtype = evalExpression(sub_action);
          addCast(result_subtype, Lexeme::subtype_numeric);
          // ld h, l
          addLdHL();
        } else {
          // ld h, 0x03
          addLdH(0x03);
        }
        // ld (ARG2), hl
        addLdiiHL(def_ARG2);

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
              result_subtype = evalExpression(sub_sub_action);
              addCast(result_subtype, Lexeme::subtype_numeric);
              // ld a, l
              addLdAL();
              // rla
              addRLA();
              // rla
              addRLA();
              // rla
              addRLA();
              // rla
              addRLA();
              // and 0xF0
              addAnd(0xF0);

              if (t >= 3) {
                // color data
                sub_sub_action = action->actions[2];
                lexeme = sub_sub_action->lexeme;
                if (lexeme->value != "ARRAY") {
                  syntaxError(
                      "Syntax not supported on SET TILE COLOR statement");
                  return;
                }
                if (i < sub_sub_action->actions.size()) {
                  sub_sub_action = sub_sub_action->actions[i];
                  lexeme = sub_sub_action->lexeme;
                  if (!(lexeme->type == Lexeme::type_literal &&
                        lexeme->subtype == Lexeme::subtype_null)) {
                    // push af
                    addPushAF();
                    // ld hl, parameter value    ; color BC data parameter
                    result_subtype = evalExpression(sub_sub_action);
                    addCast(result_subtype, Lexeme::subtype_numeric);
                    // pop af
                    addPopAF();
                    // or l
                    addOrL();
                  }
                }
              }

              // ld bc, (ARG2)
              addLdBCii(def_ARG2);
              // ld c, a
              addLdCA();
              // ld de, *i*
              addLdDE(i);
              // ld hl, (ARG)        ; tile number
              addLdHLii(def_ARG);

              // call set_tile_color ; hl = tile number, de = line number
              // (15=all), b = bank number (3=all), c = color data (FC,BC)
              addCall(def_set_tile_color);
            }
          }

        } else {
          // ld hl, parameter value    ; color FC data parameter
          result_subtype = evalExpression(sub_action);
          addCast(result_subtype, Lexeme::subtype_numeric);
          // ld a, l
          addLdAL();
          // rla
          addRLA();
          // rla
          addRLA();
          // rla
          addRLA();
          // rla
          addRLA();
          // and 0xF0
          addAnd(0xF0);

          if (t >= 3) {
            // color data
            sub_sub_action = action->actions[2];
            lexeme = sub_sub_action->lexeme;
            if (lexeme->value == "ARRAY") {
              syntaxError("Syntax not supported on SET TILE COLOR statement");
              return;
            }
            // push af
            addPushAF();
            // ld hl, parameter value    ; color BC data parameter
            result_subtype = evalExpression(sub_sub_action);
            addCast(result_subtype, Lexeme::subtype_numeric);
            // pop af
            addPopAF();
            // or l
            addOrL();
          }

          // ld bc, (ARG2)
          addLdBCii(def_ARG2);
          // ld c, a
          addLdCA();
          // ld de, 0x000F         ; all lines
          addLdDE(0x000F);
          // ld hl, (ARG)        ; tile number
          addLdHLii(def_ARG);

          // call set_tile_color ; hl = tile number, de = line number
          // (15=all), b = bank number (3=all), c = color data (FC,BC)
          addCall(def_set_tile_color);
        }

      } else {
        syntaxError("Wrong parameters count on SET TILE COLOR statement");
      }

    } else {
      syntaxError("Invalid syntax on SET TILE statement");
    }

  } else {
    syntaxError("Missing parameters on SET TILE statement");
  }
}

void Compiler::cmd_set_font() {
  ActionNode *action = current_action->actions[0], *sub_action1, *sub_action2;
  unsigned int t;
  int result_subtype;

  t = action->actions.size();
  if (t) {
    font = true;

    if (t == 1) {
      sub_action1 = action->actions[0];
      result_subtype = evalExpression(sub_action1);
      addCast(result_subtype, Lexeme::subtype_numeric);

      // ld (DAC), hl
      addLdiiHL(def_DAC);

      // ld a, 0xff                ; it means all screen banks
      addLdA(0xFF);
      // ld (ARG), a
      addLdiiA(def_ARG);

      // call cmd_setfnt
      addCall(def_cmd_setfnt);

    } else if (t == 2) {
      sub_action1 = action->actions[0];
      result_subtype = evalExpression(sub_action1);
      addCast(result_subtype, Lexeme::subtype_numeric);

      // ld (DAC), hl
      addLdiiHL(def_DAC);

      sub_action2 = action->actions[1];
      result_subtype = evalExpression(sub_action2);
      addCast(result_subtype, Lexeme::subtype_numeric);

      // ld (ARG), hl             ; screen font bank number
      addLdiiHL(def_ARG);

      // call cmd_setfnt
      addCall(def_cmd_setfnt);

    } else {
      syntaxError("Wrong number of parameters on SET FONT");
    }

  } else {
    syntaxError("SET FONT syntax error");
  }
}

void Compiler::cmd_set_sprite() {
  ActionNode *action = current_action->actions[0], *sub_action;
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
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // push hl
        addPushHL();

        // direction
        sub_action = action->actions[1];
        // ld hl, parameter value    ; parameter
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // pop de
        addPopDE();

        addCall(def_set_sprite_flip);

      } else {
        syntaxError("Wrong parameters count on SET SPRITE FLIP statement");
      }

    } else if (lexeme->value == "ROTATE") {
      if (t == 2) {
        // sprite number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; parameter
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // push hl
        addPushHL();

        // direction
        sub_action = action->actions[1];

        // ld hl, parameter value    ; parameter
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        // pop de
        addPopDE();

        addCall(def_set_sprite_rotate);

      } else {
        syntaxError("Wrong parameters count on SET SPRITE ROTATE statement");
      }

    } else if (lexeme->value == "PATTERN") {
      if (t == 2) {
        // sprite number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; sprite number
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        addLdAL();
        addPushAF();

        // sprite buffer pointer (32 bytes)
        sub_action = action->actions[1];
        sub_lexeme = sub_action->lexeme;
        if (sub_lexeme->type == Lexeme::type_identifier) {
          // ld hl, variable
          addFix(sub_lexeme);
          addLdHL(0x0000);
          result_subtype = Lexeme::subtype_numeric;
        } else {
          result_subtype = evalExpression(sub_action);
        }
        addCast(result_subtype, Lexeme::subtype_numeric);
        addPopAF();

        // call set_sprite_pattern
        //   a = sprite number
        //   hl = pointer to a 32 bytes buffer
        addCall(def_set_sprite_pattern);

      } else {
        syntaxError("Wrong parameters count on SET SPRITE PATTERN statement");
      }

    } else if (lexeme->value == "COLOR") {
      if (t == 2) {
        // sprite number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; sprite number
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        addLdAL();
        addPushAF();

        // sprite buffer pointer (16 bytes)
        sub_action = action->actions[1];
        sub_lexeme = sub_action->lexeme;
        if (sub_lexeme->type == Lexeme::type_identifier) {
          // ld hl, variable
          addFix(sub_lexeme);
          addLdHL(0x0000);
          result_subtype = Lexeme::subtype_numeric;
        } else {
          result_subtype = evalExpression(sub_action);
        }
        addCast(result_subtype, Lexeme::subtype_numeric);
        addPopAF();

        // call set_sprite_color
        //   a = sprite number
        //   hl = pointer to a 16 bytes buffer
        addCall(def_set_sprite_color);

      } else {
        syntaxError("Wrong parameters count on SET SPRITE COLOR statement");
      }

    } else {
      syntaxError("Invalid syntax on SET SPRITE statement");
    }
  } else {
    syntaxError("Missing parameters on SET SPRITE statement");
  }
}

void Compiler::cmd_set_date() {
  Lexeme* lexeme;
  ActionNode *action = current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();
  int result_subtype;

  if (t == 3) {
    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      lexeme = sub_action->lexeme;
      if (lexeme) {
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);

        switch (i) {
          case 0: {
            addPushHL();
          } break;
          case 1: {
            addLdDL();
            addPushDE();
          } break;
          case 2: {
            addPopDE();
            addLdEL();
            addPopHL();
          } break;
        }
      }
    }

    addCall(def_set_date);

  } else {
    syntaxError(
        "Wrong SET DATE parameters count.\nTry: SET DATE iYear, iMonth, "
        "iDay");
  }
}

void Compiler::cmd_set_time() {
  Lexeme* lexeme;
  ActionNode *action = current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();
  int result_subtype;

  if (t == 3) {
    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      lexeme = sub_action->lexeme;
      if (lexeme) {
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);

        switch (i) {
          case 0: {
            addLdHL();
            addPushHL();
          } break;
          case 1: {
            addPopDE();
            addLdEL();
            addPushDE();
          } break;
          case 2: {
            addLdAL();
            addPopHL();
          } break;
        }
      }
    }

    addCall(def_set_time);

  } else {
    syntaxError(
        "Wrong SET TIME parameters count.\nTry: SET TIME iHour, iMinute, "
        "iSecond");
  }
}

void Compiler::cmd_get() {
  ActionNode* action;
  Lexeme* next_lexeme;
  unsigned int t = current_action->actions.size();

  if (t == 1) {
    action = current_action->actions[0];
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
        syntaxError("Invalid GET statement");
      }
    } else {
      syntaxError("Invalid GET statement");
    }

  } else {
    syntaxError("Wrong GET parameters count");
  }
}

void Compiler::cmd_get_date() {
  Lexeme* lexeme;
  ActionNode *action = current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();

  if (t >= 3 && t <= 5) {
    addCall(def_get_date);
    addLdB(0);
    if (t > 4) {
      addExAF();
      addLdAii(0x002B);
      addLdCA();
      addPushBC();  // date format
      addExAF();
    }
    if (t > 3) {
      addLdCA();
      addPushBC();  // week
    }
    addLdCE();
    addPushBC();  // day
    addLdCD();
    addPushBC();  // month
    addPushHL();  // year

    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      lexeme = sub_action->lexeme;
      if (lexeme) {
        if (lexeme->type == Lexeme::type_identifier) {
          if (lexeme->subtype == Lexeme::subtype_numeric) {
            addVarAddress(sub_action);
            addPopDE();
            addLdiHLE();
            addIncHL();
            addLdiHLD();

          } else {
            syntaxError(
                "Invalid GET DATE parameter type.\nTry: GET DATE iYear, "
                "iMonth, iDay, iWeek, "
                "iDateFmt");
            return;
          }

        } else {
          syntaxError(
              "Invalid GET DATE parameter: it must be an integer variable.");
          return;
        }
      }
    }

  } else {
    syntaxError(
        "Wrong GET DATE parameters count.\nTry: GET DATE iYear, iMonth, "
        "iDay, "
        "iWeek, iDateFmt");
  }
}

void Compiler::cmd_get_time() {
  Lexeme* lexeme;
  ActionNode *action = current_action->actions[0], *sub_action;
  unsigned int i, t = action->actions.size();

  if (t == 3) {
    addCall(def_get_time);
    addLdB(0);
    addLdCA();
    addPushBC();  // seconds
    addLdCL();
    addPushBC();  // minutes
    addLdCH();
    addPushBC();  // hours

    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      lexeme = sub_action->lexeme;
      if (lexeme) {
        if (lexeme->type == Lexeme::type_identifier) {
          if (lexeme->subtype == Lexeme::subtype_numeric) {
            addVarAddress(sub_action);
            addPopDE();
            addLdiHLE();
            addIncHL();
            addLdiHLD();

          } else {
            syntaxError(
                "Invalid GET TIME parameter type.\nTry: GET TIME iHour, "
                "iMinute, iSecond");
            return;
          }

        } else {
          syntaxError(
              "Invalid GET TIME parameter: it must be an integer variable.");
          return;
        }
      }
    }

  } else {
    syntaxError(
        "Wrong GET TIME parameters count.\nTry: GET TIME iHour, iMinute, "
        "iSecond");
  }
}

void Compiler::cmd_get_tile() {
  ActionNode *action = current_action->actions[0], *sub_action;
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
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        addLdAL();
        addPushAF();

        // tile buffer pointer (8 bytes)
        sub_action = action->actions[1];
        sub_lexeme = sub_action->lexeme;
        if (sub_lexeme->type == Lexeme::type_identifier) {
          // ld hl, variable
          addFix(sub_lexeme);
          addLdHL(0x0000);
          result_subtype = Lexeme::subtype_numeric;
        } else {
          result_subtype = evalExpression(sub_action);
        }
        addCast(result_subtype, Lexeme::subtype_numeric);
        addPopAF();

        // call get_tile_pattern
        //   a = tile number
        //   hl = pointer to an 8 bytes buffer
        addCall(def_get_tile_pattern);

      } else {
        syntaxError("Wrong parameters count on GET TILE PATTERN statement");
      }

    } else if (lexeme->value == "COLOR") {
      if (t == 2) {
        // tile number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; tile number
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        addLdAL();
        addPushAF();

        // tile buffer pointer (8 bytes)
        sub_action = action->actions[1];
        sub_lexeme = sub_action->lexeme;
        if (sub_lexeme->type == Lexeme::type_identifier) {
          // ld hl, variable
          addFix(sub_lexeme);
          addLdHL(0x0000);
          result_subtype = Lexeme::subtype_numeric;
        } else {
          result_subtype = evalExpression(sub_action);
        }
        addCast(result_subtype, Lexeme::subtype_numeric);
        addPopAF();

        // call get_tile_color
        //   a = sprite number
        //   hl = pointer to an 8 bytes buffer
        addCall(def_get_tile_color);

      } else {
        syntaxError("Wrong parameters count on GET TILE COLOR statement");
      }

    } else {
      syntaxError("Invalid syntax on GET TILE statement");
    }
  } else {
    syntaxError("Missing parameters on GET TILE statement");
  }
}

void Compiler::cmd_get_sprite() {
  ActionNode *action = current_action->actions[0], *sub_action;
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
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        addLdAL();
        addPushAF();

        // sprite buffer pointer (32 bytes)
        sub_action = action->actions[1];
        sub_lexeme = sub_action->lexeme;
        if (sub_lexeme->type == Lexeme::type_identifier) {
          // ld hl, variable
          addFix(sub_lexeme);
          addLdHL(0x0000);
          result_subtype = Lexeme::subtype_numeric;
        } else {
          result_subtype = evalExpression(sub_action);
        }
        addCast(result_subtype, Lexeme::subtype_numeric);
        addPopAF();

        // call get_sprite_pattern
        //   a = sprite number
        //   hl = pointer to a 32 bytes buffer
        addCall(def_get_sprite_pattern);

      } else {
        syntaxError("Wrong parameters count on GET SPRITE PATTERN statement");
      }

    } else if (lexeme->value == "COLOR") {
      if (t == 2) {
        // sprite number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; sprite number
        result_subtype = evalExpression(sub_action);
        addCast(result_subtype, Lexeme::subtype_numeric);
        addLdAL();
        addPushAF();

        // sprite buffer pointer (16 bytes)
        sub_action = action->actions[1];
        sub_lexeme = sub_action->lexeme;
        if (sub_lexeme->type == Lexeme::type_identifier) {
          // ld hl, variable
          addFix(sub_lexeme);
          addLdHL(0x0000);
          result_subtype = Lexeme::subtype_numeric;
        } else {
          result_subtype = evalExpression(sub_action);
        }
        addCast(result_subtype, Lexeme::subtype_numeric);
        addPopAF();

        // call get_sprite_color
        //   a = sprite number
        //   hl = pointer to a 16 bytes buffer
        addCall(def_get_sprite_color);

      } else {
        syntaxError("Wrong parameters count on GET SPRITE COLOR statement");
      }

    } else {
      syntaxError("Invalid syntax on GET SPRITE statement");
    }
  } else {
    syntaxError("Missing parameters on GET SPRITE statement");
  }
}

void Compiler::cmd_on() {
  ActionNode* action;
  Lexeme* next_lexeme;
  unsigned int t = current_action->actions.size();

  if (t) {
    action = current_action->actions[0];
    next_lexeme = action->lexeme;

    if (next_lexeme->type == Lexeme::type_keyword &&
        next_lexeme->value == "ERROR") {
      cmd_on_error();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "INTERVAL") {
      cmd_on_interval();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "KEY") {
      cmd_on_key();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "SPRITE") {
      cmd_on_sprite();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "STOP") {
      cmd_on_stop();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "STRIG") {
      cmd_on_strig();
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "INDEX") {
      cmd_on_goto_gosub();
    } else {
      syntaxError("Invalid ON statement");
    }

  } else {
    syntaxError("Empty ON statement");
  }
}

void Compiler::cmd_on_error() {
  syntaxError("Not implemented yet");
}

void Compiler::cmd_on_interval() {
  ActionNode *action, *sub_action, *parm_action;
  Lexeme *lexeme, *parm_lexeme;
  unsigned int t;
  int result_subtype;

  action = current_action->actions[0];
  t = action->actions.size();

  if (t == 2) {
    // INDEX VARIABLE

    sub_action = action->actions[0];
    lexeme = sub_action->lexeme;
    if (lexeme->value != "INDEX") {
      syntaxError("Interval index is missing in ON INTERVAL");
      return;
    }
    if (sub_action->actions.size() != 1) {
      syntaxError("Wrong parameter count in interval index from ON INTERVAL");
      return;
    }

    parm_action = sub_action->actions[0];

    // ld hl, variable
    result_subtype = evalExpression(parm_action);

    addCast(result_subtype, Lexeme::subtype_numeric);

    // di
    addDI();
    //   ld (0xFCA0), hl   ; INTVAL
    addLdiiHL(0xFCA0);
    //   xor a
    addXorA();
    //   ld (0xFC7F), a    ; ON INTERVAL STATE (0=off, 1=on)
    addLdiiA(0xFC7F);
    //   ld (0xFCA3), a    ; INTCNT - initialize with zero (2 bytes)
    addLdiiA(0xFCA3);
    addLdiiA(0xFCA4);
    // ei
    addEI();

    // GOSUB

    sub_action = action->actions[1];
    lexeme = sub_action->lexeme;
    if (lexeme->value != "GOSUB") {
      syntaxError("GOSUB is missing in ON INTERVAL");
      return;
    }
    if (sub_action->actions.size() != 1) {
      syntaxError("Wrong parameter count in GOSUB from ON INTERVAL");
      return;
    }

    parm_action = sub_action->actions[0];
    parm_lexeme = parm_action->lexeme;

    if (parm_lexeme->type == Lexeme::type_literal &&
        parm_lexeme->subtype == Lexeme::subtype_numeric) {
      if (opts->megaROM) {
        // ld hl, GOSUB ADDRESS
        addFix(parm_lexeme->value);
        addLdHLmegarom();
        // ld (0xFC80), hl                ; INTERVAL ADDRESS
        addLdiiHL(0xFC80);
        // ld (MR_TRAP_SEGMS+17), a       ; INTERVAL segment
        addLdiiA(def_MR_TRAP_SEGMS + 17);

      } else {
        // ld hl, GOSUB ADDRESS
        addFix(parm_lexeme->value);
        addLdHL(0x0000);
        // ld (0xFC80), hl   ; GOSUB ADDRESS
        addLdiiHL(0xFC80);
      }

    } else {
      syntaxError("Invalid GOSUB parameter in ON INTERVAL");
      return;
    }

  } else {
    syntaxError("ON INTERVAL with empty parameters");
  }
}

void Compiler::cmd_interval() {
  ActionNode* action;
  Lexeme* next_lexeme;
  unsigned int t = current_action->actions.size();

  if (t == 1) {
    action = current_action->actions[0];
    next_lexeme = action->lexeme;

    // ld hl, 0xFC7F    ; INTERVAL state
    addLdHL(0xfc7f);

    if (next_lexeme->type == Lexeme::type_keyword &&
        next_lexeme->value == "ON") {
      // call 0x6c89   ; xbasic turn on trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_ON);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "OFF") {
      // call 0x6c9c   ; xbasic turn off trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_OFF);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "STOP") {
      // call 0x6ca5   ; xbasic turn stop trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_STOP);
    } else {
      syntaxError("Invalid INTERVAL statement");
    }

  } else {
    syntaxError("Empty INTERVAL statement");
  }
}

void Compiler::cmd_stop() {
  ActionNode* action;
  Lexeme* next_lexeme;
  unsigned int t = current_action->actions.size();

  if (t == 1) {
    action = current_action->actions[0];
    next_lexeme = action->lexeme;

    // ld hl, 0xFC6A    ; STOP state
    addLdHL(0xFC6A);

    if (next_lexeme->type == Lexeme::type_keyword &&
        next_lexeme->value == "ON") {
      // call 0x6c89   ; xbasic turn on trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_ON);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "OFF") {
      // call 0x6c9c   ; xbasic turn off trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_OFF);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "STOP") {
      // call 0x6ca5   ; xbasic turn stop trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_STOP);
    } else {
      syntaxError("Invalid STOP statement");
    }

  } else if (t == 0) {
    cmd_end(false);  //! jump to the real END statement
  } else {
    syntaxError("Wrong number of parameters in STOP");
  }
}

void Compiler::cmd_sprite() {
  ActionNode* action;
  Lexeme* next_lexeme;
  unsigned int t = current_action->actions.size();

  if (t == 1) {
    action = current_action->actions[0];
    next_lexeme = action->lexeme;

    if (next_lexeme->value == "LOAD") {
      current_action = action;
      cmd_sprite_load();
      return;
    }

    // ld hl, 0xFC6D   ; SPRITE state
    addLdHL(0xFC6D);

    if (next_lexeme->type == Lexeme::type_keyword &&
        next_lexeme->value == "ON") {
      // call 0x6c89   ; xbasic turn on trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_ON);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "OFF") {
      // call 0x6c9c   ; xbasic turn off trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_OFF);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "STOP") {
      // call 0x6ca5   ; xbasic turn stop trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_STOP);
    } else {
      syntaxError("Invalid SPRITE statement");
    }

  } else {
    syntaxError("Empty SPRITE statement");
  }
}

void Compiler::cmd_sprite_load() {
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t) {
    if (t > 1) {
      syntaxError("SPRITE LOAD with excess of parameters");
      return;
    }

    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      result_subtype = evalExpression(action);
      addCast(result_subtype, Lexeme::subtype_numeric);

      // ld (DAC), hl
      addLdiiHL(def_DAC);
      // call cmd_wrtspr                    ; tiny sprite loader
      addCall(def_cmd_wrtspr);
    }

  } else {
    syntaxError("SPRITE LOAD with empty parameters");
  }
}

void Compiler::cmd_key() {
  ActionNode *action1, *action2;
  Lexeme* next_lexeme;
  unsigned int t = current_action->actions.size();
  int result_subtype;

  if (t == 1) {
    action1 = current_action->actions[0];
    next_lexeme = action1->lexeme;

    if (next_lexeme->type == Lexeme::type_keyword &&
        next_lexeme->value == "ON") {
      // call 0x00CF   ; DSPFNK - (0xF3DE = CNSDFG: function keys
      // presentation)
      addCall(0x00CF);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "OFF") {
      // call 0x00CC   ; ERAFNK
      addCall(0x00CC);
    } else {
      syntaxError("Invalid KEY statement");
    }

  } else if (t == 2) {
    action2 = current_action->actions[1];
    next_lexeme = action2->lexeme;

    // ld hl, key number
    action1 = current_action->actions[0];
    result_subtype = evalExpression(action1);
    addCast(result_subtype, Lexeme::subtype_numeric);

    // dec hl
    addDecHL();

    if ((next_lexeme->type == Lexeme::type_keyword &&
         next_lexeme->subtype == Lexeme::subtype_function) ||
        (next_lexeme->type == Lexeme::type_operator &&
         next_lexeme->value == "+") ||
        (next_lexeme->subtype == Lexeme::subtype_string)) {
      // add hl, hl
      addAddHLHL();
      // add hl, hl
      addAddHLHL();
      // add hl, hl
      addAddHLHL();
      // add hl, hl      ; key number * 16
      addAddHLHL();
      // ld de, 0xF87F   ; FNKSTR (10 x 16 bytes)
      addLdDE(0xf87f);
      // add hl, de
      addAddHLDE();

      // push hl
      addPushHL();
      //   ld a, 32        ; space
      addLdA(0x20);
      //   ld (hl), a
      addLdiHLA();
      //   ld e, l
      addLdEL();
      //   ld d, h
      addLdDH();
      //   inc de
      addIncDE();
      //   ld bc, 15
      addLdBC(0x000F);
      //   ldir
      addLDIR();

      //   ld hl, variable address
      result_subtype = evalExpression(action2);
      addCast(result_subtype, Lexeme::subtype_string);

      //   xor a
      addXorA();
      //   ld c, (hl)
      addLdCiHL();
      //   ld b, a
      addLdBA();
      //   inc hl
      addIncHL();

      // pop de
      addPopDE();

      // or c
      addOrC();
      // jr z, $+3
      addJrZ(0x02);
      //   ldir
      addLDIR();
      // ld (de), a
      addLdiDEA();
      // call 0x00C9    ; FNKSB
      addCall(0x00C9);

    } else {
      // ld e, l
      addLdEL();
      // ld d, h
      addLdDH();
      // add hl, de
      addAddHLDE();
      // add hl, de       ; key number * 3
      addAddHLDE();

      // ld de, 0xFC4C    ; KEY state position = key number * 3 + 0xFC4C
      addLdDE(0xfc4c);

      // add hl, de
      addAddHLDE();

      if (next_lexeme->type == Lexeme::type_keyword &&
          next_lexeme->value == "ON") {
        // call 0x6c89   ; xbasic turn on trap (hl=trap state address)
        addCall(def_XBASIC_TRAP_ON);
      } else if (next_lexeme->type == Lexeme::type_keyword &&
                 next_lexeme->value == "OFF") {
        // call 0x6c9c   ; xbasic turn off trap (hl=trap state address)
        addCall(def_XBASIC_TRAP_OFF);
      } else if (next_lexeme->type == Lexeme::type_keyword &&
                 next_lexeme->value == "STOP") {
        // call 0x6ca5   ; xbasic turn stop trap (hl=trap state address)
        addCall(def_XBASIC_TRAP_STOP);
      } else {
        syntaxError("Invalid KEY statement");
      }
    }

  } else {
    syntaxError("Empty KEY statement");
  }
}

void Compiler::cmd_strig() {
  ActionNode *action1, *action2;
  Lexeme* next_lexeme;
  unsigned int t = current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    action2 = current_action->actions[1];
    next_lexeme = action2->lexeme;

    // ld hl, strig number
    action1 = current_action->actions[0];
    result_subtype = evalExpression(action1);
    addCast(result_subtype, Lexeme::subtype_numeric);

    // dec hl
    // addDecHL();

    // ld e, l
    addLdEL();
    // ld d, h
    addLdDH();
    // add hl, de
    addAddHLDE();
    // add hl, de       ; strig number * 3
    addAddHLDE();

    // ld de, 0xFC70    ; STRIG state position = key number * 3 + 0xFC70
    addLdDE(0xFC70);

    // add hl, de
    addAddHLDE();

    if (next_lexeme->type == Lexeme::type_keyword &&
        next_lexeme->value == "ON") {
      // call 0x6c89   ; xbasic turn on trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_ON);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "OFF") {
      // call 0x6c9c   ; xbasic turn off trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_OFF);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "STOP") {
      // call 0x6ca5   ; xbasic turn stop trap (hl=trap state address)
      addCall(def_XBASIC_TRAP_STOP);
    } else {
      syntaxError("Invalid STRIG statement");
    }

  } else {
    syntaxError("Wrong number of parameters in STRIG statement");
  }
}

bool Compiler::addCheckTraps() {
  if (parser->getHasTraps()) {
    // call 0x6c25   ; xbasic check traps
    addCall(def_XBASIC_TRAP_CHECK);
  }

  return parser->getHasTraps();
}

void Compiler::cmd_on_key() {
  ActionNode *action, *sub_action;
  Lexeme *lexeme, *sub_lexeme;
  unsigned int i, t;

  action = current_action->actions[0];
  if (action->actions.size() != 1) {
    syntaxError("Wrong parameters in ON KEY");
    return;
  }

  action = action->actions[0];
  lexeme = action->lexeme;
  if (lexeme->value != "GOSUB") {
    syntaxError("GOSUB parameters is missing in ON KEY");
    return;
  }
  t = action->actions.size();

  if (t) {
    // GOSUB LIST

    // ld hl, 0xFC4D    ; KEY first GOSUB position = 0xFC4C+1
    addLdHL(0xfc4D);

    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      sub_lexeme = sub_action->lexeme;

      if (sub_lexeme->type == Lexeme::type_literal &&
          sub_lexeme->subtype == Lexeme::subtype_numeric) {
        if (opts->megaROM) {
          // push hl
          addPushHL();
          //   ld hl, GOSUB ADDRESS
          addFix(sub_lexeme->value);
          addLdHLmegarom();
          //   ld (MR_TRAP_SEGMS), a       ; KEY segment
          addLdiiA(def_MR_TRAP_SEGMS + i);
          //   ex de, hl
          addExDEHL();
          // pop hl
          addPopHL();

        } else {
          // ld de, call address
          addFix(sub_lexeme->value);
          addLdDE(0x0000);
        }

      } else {
        // ld hl, 0x368D   ; dummy bios RET address
        addLdHL(def_XBASIC_DUMMY_RET);
      }

      // ld (hl), e
      addLdiHLE();
      // inc hl
      addIncHL();
      // ld (hl), d
      addLdiHLD();
      // inc hl
      addIncHL();
      // inc hl
      addIncHL();
    }

  } else {
    syntaxError("ON KEY with empty parameters");
  }
}

void Compiler::cmd_on_sprite() {
  ActionNode *action, *sub_action;
  Lexeme *lexeme, *sub_lexeme;
  unsigned int t, i = 0;

  action = current_action->actions[0];
  if (action->actions.size() != 1) {
    syntaxError("Wrong parameters in ON SPRITE");
    return;
  }

  action = action->actions[0];
  lexeme = action->lexeme;
  if (lexeme->value != "GOSUB") {
    syntaxError("GOSUB parameters is missing in ON SPRITE");
    return;
  }
  t = action->actions.size();

  if (t == 1) {
    // GOSUB address

    sub_action = action->actions[0];
    sub_lexeme = sub_action->lexeme;

    if (sub_lexeme->type == Lexeme::type_literal &&
        sub_lexeme->subtype == Lexeme::subtype_numeric) {
      if (opts->megaROM) {
        // push hl
        addPushHL();
        //   ld hl, GOSUB ADDRESS
        addFix(sub_lexeme->value);
        addLdHLmegarom();
        //   ld (MR_TRAP_SEGMS+11), a       ; SPRITE segment
        addLdiiA(def_MR_TRAP_SEGMS + 11 + i);
        //   ex de, hl
        addExDEHL();
        // pop hl
        addPopHL();

      } else {
        // ld hl, call address
        addFix(sub_lexeme->value);
        addLdHL(0x0000);
      }

    } else {
      // ld hl, 0x368D   ; dummy bios RET address
      addLdHL(def_XBASIC_DUMMY_RET);
    }

    // ld (0xFC6E), hl     ; STOP GOSUB position = 0xFC6D+1
    addLdiiHL(0xFC6E);

  } else {
    syntaxError("ON SPRITE with wrong count of parameters");
  }
}

void Compiler::cmd_on_stop() {
  ActionNode *action, *sub_action;
  Lexeme *lexeme, *sub_lexeme;
  unsigned int t, i = 0;

  action = current_action->actions[0];
  if (action->actions.size() != 1) {
    syntaxError("Wrong parameters in ON STOP");
    return;
  }

  action = action->actions[0];
  lexeme = action->lexeme;
  if (lexeme->value != "GOSUB") {
    syntaxError("GOSUB parameters is missing in ON STOP");
    return;
  }
  t = action->actions.size();

  if (t == 1) {
    // GOSUB address

    sub_action = action->actions[0];
    sub_lexeme = sub_action->lexeme;

    if (sub_lexeme->type == Lexeme::type_literal &&
        sub_lexeme->subtype == Lexeme::subtype_numeric) {
      if (opts->megaROM) {
        // push hl
        addPushHL();
        //   ld hl, GOSUB ADDRESS
        addFix(sub_lexeme->value);
        addLdHLmegarom();
        //   ld (MR_TRAP_SEGMS+10), a       ; STOP segment
        addLdiiA(def_MR_TRAP_SEGMS + 10 + i);
        //   ex de, hl
        addExDEHL();
        // pop hl
        addPopHL();

      } else {
        // ld hl, call address
        addFix(sub_lexeme->value);
        addLdHL(0x0000);
      }

    } else {
      // ld hl, 0x368D   ; dummy bios RET address
      addLdHL(def_XBASIC_DUMMY_RET);
    }

    // ld (0xFC6B), hl     ; STOP GOSUB position = 0xFC6A+1
    addLdiiHL(0xfc6B);

  } else {
    syntaxError("ON STOP with wrong count of parameters");
  }
}

void Compiler::cmd_on_strig() {
  ActionNode *action, *sub_action;
  Lexeme *lexeme, *sub_lexeme;
  unsigned int i, t;

  action = current_action->actions[0];
  if (action->actions.size() != 1) {
    syntaxError("Wrong parameters in ON STRIG");
    return;
  }

  action = action->actions[0];
  lexeme = action->lexeme;
  if (lexeme->value != "GOSUB") {
    syntaxError("GOSUB parameters is missing in ON STRIG");
    return;
  }
  t = action->actions.size();

  if (t) {
    // GOSUB LIST

    // ld hl, 0xFC71    ; STRIG first GOSUB position = 0xFC70+1
    addLdHL(0xFC71);

    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      sub_lexeme = sub_action->lexeme;

      if (sub_lexeme->type == Lexeme::type_literal &&
          sub_lexeme->subtype == Lexeme::subtype_numeric) {
        if (opts->megaROM) {
          // push hl
          addPushHL();
          //   ld hl, GOSUB ADDRESS
          addFix(sub_lexeme->value);
          addLdHLmegarom();
          //   ld (MR_TRAP_SEGMS+12), a       ; STRIG segment
          addLdiiA(def_MR_TRAP_SEGMS + 12 + i);
          //   ex de, hl
          addExDEHL();
          // pop hl
          addPopHL();

        } else {
          // ld de, call address
          addFix(sub_lexeme->value);
          addLdDE(0x0000);
        }

      } else {
        // ld de, 0x368D   ; dummy bios RET address
        addLdDE(def_XBASIC_DUMMY_RET);
      }

      // ld (hl), e
      addLdiHLE();
      // inc hl
      addIncHL();
      // ld (hl), d
      addLdiHLD();
      // inc hl
      addIncHL();
      // inc hl
      addIncHL();
    }

  } else {
    syntaxError("ON STRIG with empty parameters");
  }
}

void Compiler::cmd_on_goto_gosub() {
  ActionNode *action, *sub_action;
  Lexeme *lexeme, *sub_lexeme;
  FixNode* mark;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    // INDEX VARIABLE
    // ld hl, variable

    action = current_action->actions[0];
    if (action->actions.size() == 0) {
      syntaxError("Empty parameter in ON GOTO/GOSUB");
      return;
    }

    sub_action = action->actions[0];

    result_subtype = evalExpression(sub_action);

    addCast(result_subtype, Lexeme::subtype_numeric);

    // GOTO / GOSUB LIST

    action = current_action->actions[1];
    lexeme = action->lexeme;
    t = action->actions.size();

    // ld a, l
    addLdAL();

    if (opts->megaROM) {
      // ld (TEMP), a
      addLdiiA(def_TEMP);
    }

    // and a
    addAndA();
    // jp z, address
    mark = addMark();
    addJpZ(0x0000);

    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      sub_lexeme = sub_action->lexeme;

      if (sub_lexeme->type == Lexeme::type_literal &&
          sub_lexeme->subtype == Lexeme::subtype_numeric) {
        // Trim leading zeros
        while (sub_lexeme->value.find("0") == 0 &&
               sub_lexeme->value.size() > 1) {
          sub_lexeme->value.erase(0, 1);
        }

        if (lexeme->value == "GOTO") {
          if (opts->megaROM) {
            // ld a, (TEMP)
            addLdAii(def_TEMP);
          }

          // dec a
          addDecA();

          if (opts->megaROM) {
            // ld (TEMP), a
            addLdiiA(def_TEMP);
          }

          // jp z, address
          addFix(sub_lexeme->value);
          addJpZ(0x0000);

        } else {
          if (opts->megaROM) {
            // ld a, (TEMP)
            addLdAii(def_TEMP);
          }

          // dec a
          addDecA();

          if (opts->megaROM) {
            // ld (TEMP), a
            addLdiiA(def_TEMP);
          }

          if (opts->megaROM) {
            // jr nz, $+25
            addJrNZ(24);
          } else {
            // jr nz, $+7
            addJrNZ(0x06);
          }
          //   call address
          addFix(sub_lexeme->value);
          addCall(0x0000);
          //   jp address
          addFix(mark->symbol);
          addJp(0x0000);
        }

      } else {
        if (opts->megaROM) {
          // ld a, (TEMP)
          addLdAii(def_TEMP);
        }

        // dec a
        addDecA();

        if (opts->megaROM) {
          // ld (TEMP), a
          addLdiiA(def_TEMP);
        }
      }
    }

    mark->symbol->address = code_pointer;

  } else {
    syntaxError("ON GOTO/GOSUB with empty parameters");
  }
}

void Compiler::cmd_swap() {
  Lexeme *lexeme1, *lexeme2;
  ActionNode *action1, *action2;
  unsigned int t = current_action->actions.size();

  if (t == 2) {
    action1 = current_action->actions[0];
    lexeme1 = action1->lexeme;
    if (lexeme1->type != Lexeme::type_identifier) {
      syntaxError("Invalid parameter type in SWAP (1st)");
      return;
    }

    action2 = current_action->actions[1];
    lexeme2 = action2->lexeme;
    if (lexeme2->type != Lexeme::type_identifier) {
      syntaxError("Invalid parameter type in SWAP (2nd)");
      return;
    }

    if (lexeme1->subtype == lexeme2->subtype) {
      // ld hl, variable
      addVarAddress(action1);
      // push hl
      addPushHL();
      // ld hl, variable
      addVarAddress(action2);
      // pop de
      addByteOptimized(0xD1);

      if (lexeme1->subtype == Lexeme::subtype_numeric) {
        // call 0x6bf5    ; xbasic SWAP integers (in: hl=var1, de=var2)
        addCall(def_XBASIC_SWAP_INTEGER);
      } else if (lexeme1->subtype == Lexeme::subtype_string) {
        // call 0x6bf9    ; xbasic SWAP strings (in: hl=var1, de=var2)
        addCall(def_XBASIC_SWAP_STRING);
      } else {
        // call 0x6bfd    ; xbasic SWAP floats (in: hl=var1, de=var2)
        addCall(def_XBASIC_SWAP_FLOAT);
      }

    } else {
      syntaxError("Parameters type mismatch in SWAP");
    }

  } else {
    syntaxError("Invalid SWAP parameters");
  }
}

void Compiler::cmd_wait() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t == 2 || t == 3) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme) {
        result_subtype = evalExpression(action);

        // cast
        addCast(result_subtype, Lexeme::subtype_numeric);

        switch (i) {
          case 0: {
            // push hl
            addPushHL();
          } break;

          case 1: {
            if (t == 3) {
              // ld a, l
              addLdAL();
              // push af
              addPushAF();
            } else {
              // ld d, l
              addLdDL();
              // pop bc
              addPopBC();
              //   in a,(c)
              addWord(0xED, 0x78);
              //   and d
              addAndD();
              // jr z, $-4
              addJrZ(0xFB);
            }
          } break;

          case 2: {
            // ld h, l
            addLdHL();
            // pop de
            addPopDE();
            // pop bc
            addPopBC();
            //   in a,(c)
            addWord(0xED, 0x78);
            //   xor h
            addXorH();
            //   and d
            addAndD();
            // jr z, $-5
            addJrZ(0xFA);

          } break;
        }
      }
    }

  } else {
    syntaxError("Invalid WAIT parameters");
  }
}

void Compiler::cmd_data() {
  if (!current_action->actions.size()) {
    syntaxError("DATA with empty parameters");
  }
}

void Compiler::cmd_idata() {
  if (!current_action->actions.size()) {
    syntaxError("IDATA with empty parameters");
  }
}

void Compiler::cmd_read() {
  ActionNode* action;
  Lexeme* lexeme;
  unsigned int i, t = current_action->actions.size();

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->type != Lexeme::type_identifier) {
        syntaxError("Invalid READ parameter type");
        return;
      }

      // call read
      addCall(def_XBASIC_READ);

      addCast(Lexeme::subtype_string, lexeme->subtype);

      // do assignment

      addAssignment(action);
    }

  } else {
    syntaxError("READ with empty parameters");
  }
}

void Compiler::cmd_iread() {
  ActionNode* action;
  Lexeme* lexeme;
  unsigned int i, t = current_action->actions.size();

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->type != Lexeme::type_identifier) {
        syntaxError("Invalid IREAD parameter type");
        return;
      }

      // call XBASIC_IREAD
      addCall(def_XBASIC_IREAD);

      addCast(Lexeme::subtype_numeric, lexeme->subtype);

      // do assignment

      addAssignment(action);
    }

  } else {
    syntaxError("IREAD with empty parameters");
  }
}

void Compiler::cmd_resume() {
  syntaxError("RESUME statement not supported in compiled mode");
}

void Compiler::cmd_restore() {
  ActionNode* action;
  Lexeme* lexeme;
  int t = current_action->actions.size();
  int result_subtype;

  if (t == 0) {
    // ld hl, 0
    addLdHL(0x0000);
    // call xbasic_restore
    addCall(def_XBASIC_RESTORE);  // standard BASIC RESTORE statement

  } else if (t == 1) {
    action = current_action->actions[0];

    lexeme = action->lexeme;
    if (lexeme) {
      result_subtype = evalExpression(action);

      // cast
      addCast(result_subtype, Lexeme::subtype_numeric);

      // call restore
      addCall(def_XBASIC_RESTORE);  // standard BASIC RESTORE statement
    }

  } else {
    syntaxError("RESTORE with wrong number of parameters");
  }
}

void Compiler::cmd_irestore() {
  ActionNode* action;
  Lexeme* lexeme;
  int t = current_action->actions.size();
  int result_subtype;

  if (t == 0) {
    // ld hl, (DATLIN)
    addLdHLii(def_DATLIN);
    // ld (DATPTR), hl
    addLdiiHL(def_DATPTR);

  } else if (t == 1) {
    action = current_action->actions[0];

    lexeme = action->lexeme;
    if (lexeme) {
      result_subtype = evalExpression(action);

      // cast
      addCast(result_subtype, Lexeme::subtype_numeric);

      // ex de, hl
      addExDEHL();
      // ld hl, (DATLIN)
      addLdHLii(def_DATLIN);
      // add hl, de
      addAddHLDE();
      // ld (DATPTR), hl
      addLdiiHL(def_DATPTR);
    }

  } else {
    syntaxError("IRESTORE with wrong number of parameters");
  }
}

void Compiler::cmd_out() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        result_subtype = evalExpression(action);

        // cast
        addCast(result_subtype, Lexeme::subtype_numeric);

        if (i == 0) {
          // push hl
          addPushHL();
        } else {
          // pop bc
          addPopBC();
          // out (c), l
          addWord(0xED, 0x69);
        }
      }
    }

  } else {
    syntaxError("Invalid OUT parameters");
  }
}

void Compiler::cmd_poke() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        result_subtype = evalExpression(action);

        // cast
        addCast(result_subtype, Lexeme::subtype_numeric);

        if (i == 0) {
          // push hl
          addPushHL();
        } else {
          // ex de, hl
          addExDEHL();
          // pop hl
          addPopHL();
          // ld (hl), e
          addLdiHLE();
        }
      }
    }

  } else {
    syntaxError("Invalid POKE parameters");
  }
}

void Compiler::cmd_ipoke() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        result_subtype = evalExpression(action);

        // cast
        addCast(result_subtype, Lexeme::subtype_numeric);

        if (i == 0) {
          // push hl
          addPushHL();
        } else {
          // ex de, hl
          addExDEHL();
          // pop hl
          addPopHL();
          // ld (hl), e
          addLdiHLE();
          // inc hl
          addIncHL();
          // ld (hl), d
          addLdiHLD();
        }
      }
    }

  } else {
    syntaxError("Invalid IPOKE parameters");
  }
}

void Compiler::cmd_vpoke() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        result_subtype = evalExpression(action);

        // cast
        addCast(result_subtype, Lexeme::subtype_numeric);

        if (i == 0) {
          // push hl
          addPushHL();
        } else {
          // ld a, l
          addLdAL();
          // pop hl
          addPopHL();
          // call 0x70b5    ; xbasic VPOKE (in: hl=address, a=byte)
          addCall(def_XBASIC_VPOKE);
        }
      }
    }

  } else {
    syntaxError("Invalid VPOKE parameters");
  }
}

void Compiler::cmd_file() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int t = current_action->actions.size();
  string filename;

  if (t == 1) {
    action = current_action->actions[0];
    lexeme = action->lexeme;

    if (lexeme->type == Lexeme::type_literal &&
        lexeme->subtype == Lexeme::subtype_string) {
      lexeme->name = "FILE";
      filename = removeQuotes(lexeme->value);
      resourceManager.addFile(filename, opts->inputPath);
    } else {
      error_message = "Invalid parameter in FILE keyword";
    }

  } else {
    syntaxError("Wrong FILE parameters count");
  }
}

void Compiler::cmd_text() {
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int t = current_action->actions.size();

  if (t == 1) {
    action = current_action->actions[0];
    lexeme = action->lexeme;

    if (lexeme->type == Lexeme::type_literal &&
        lexeme->subtype == Lexeme::subtype_string) {
      lexeme->name = "TEXT";
      resourceManager.addText(lexeme->value);
    } else {
      error_message = "Invalid parameter in TEXT keyword";
    }

  } else {
    syntaxError("Wrong TEXT parameters count");
  }
}

void Compiler::cmd_call() {
  ActionNode* action;
  Lexeme* lexeme;
  unsigned int i, t = current_action->actions.size();

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->value == "TURBO") {
        // not need anymore... he he he...

      } else if (lexeme->value == "SYSTEM") {
        // ld hl, data address
        addVarAddress(action);
        // ld a, (hl)      ; first character
        addLdAiHL();
        // ld ix, (CALL)    ; CALL
        addLdIXii(def_CALL_STMT);
        // call CALBAS
        addCall(def_CALBAS);
        // ei
        addEI();

      } else {
        // todo: concatenate CALL statement  into a literal string
        //       and run it with BASIC like above
        syntaxError("CALL statement invalid");
        return;
      }
    }

  } else {
    syntaxError("CALL with empty parameters");
  }
}

void Compiler::cmd_maxfiles() {
  ActionNode* action;
  unsigned int t = current_action->actions.size();
  int result_subtype;
  FixNode* mark;

  if (t) {
    // xor a
    // addXorA();
    // ld (NLONLY), a
    // addLdiiA(def_NLONLY);
    // ld (DSKDIS), a
    // addLdiiA(0xFD99);

    action = current_action->actions[0];
    result_subtype = evalExpression(action);
    addCast(result_subtype, Lexeme::subtype_numeric);

    // ld a, (MAXFIL)
    addLdAii(def_MAXFIL);
    // cp l
    addCpL();
    // jp z, $                ; skip if equal
    mark = addMark();
    addJpZ(0x0000);

    // ld a, l
    addLdAL();

    // ld (MAXFIL), a
    addLdiiA(def_MAXFIL);

    // push af
    addPushAF();

    addEnableBasicSlot();

    // ld hl, fake empty line
    addLdHL(def_ENDPRG);

    // call CLOSE_ALL
    addCall(def_CLOSE_ALL);

    // pop af
    addPopAF();

    // call MAXFILES
    addCall(def_MAXFILES);

    // call 0x7304			; end printeroutput (basic interpreter
    // function)
    addCall(0x7304);
    // call 0x4AFF			; return interpreter output to screen
    // (basic interpreter function)
    addCall(0x4AFF);

    addDisableBasicSlot();

    // ; restore stack state
    // ld bc, 16
    addLdBC(0x0010);
    // ld (TEMP), sp
    addLdiiSP(def_TEMP);
    // ld hl, (TEMP)
    addLdHLii(def_TEMP);
    // xor a
    addXorA();
    // sbc hl, bc
    addSbcHLBC();
    // ld sp,hl
    addLdSPHL();
    // ex de,hl
    addExDEHL();
    // ld hl, (SAVSTK)
    addLdHLii(def_SAVSTK);
    // ldir
    addLDIR();
    // ld (SAVSTK), sp
    addLdiiSP(def_SAVSTK);

    mark->symbol->address = code_pointer;

  } else {
    syntaxError("Empty MAXFILES assignment");
  }
}

void Compiler::cmd_open() {
  ActionNode* action;
  Lexeme* lexeme;
  unsigned int i, t = current_action->actions.size();
  int result_subtype, state = 0;
  FixNode* mark;
  bool has[4];

  for (i = 0; i < 4; i++) has[i] = false;

  file_support = true;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;

      switch (state) {
        // file name
        case 0: {
          has[state] = true;
          state = 1;
          result_subtype = evalExpression(action);
          addCast(result_subtype, Lexeme::subtype_string);

          // push hl
          addPushHL();
        } break;

        // FOR
        case 1: {
          has[state] = true;
          state = 2;
          if (lexeme->value == "OUT") {
            // ld a, 2     ; output mode
            addLdA(0x02);
          } else if (lexeme->value == "APP") {
            // ld a, 8     ; append mode
            addLdA(0x08);
          } else if (lexeme->value == "INPUT") {
            // ld a, 1     ; input mode
            addLdA(0x01);
          } else {
            // ld a, 4     ; random mode
            addLdA(0x04);
            i--;
          }
          // push af
          addPushAF();
        } break;

        // AS
        case 2: {
          has[state] = true;
          state = 3;
          result_subtype = evalExpression(action);
          addCast(result_subtype, Lexeme::subtype_numeric);
          // push hl
          addPushHL();
        } break;

        // LEN
        case 3: {
          has[state] = true;
          state = 4;

          result_subtype = evalExpression(action);
          addCast(result_subtype, Lexeme::subtype_numeric);
        } break;
      }
    }

    addEnableBasicSlot();

    // LEN
    if (!has[3]) {
      // ld hl, 256  ; default record size
      addLdHL(0x0100);
    }
    // ld (RECSIZ), hl
    addLdiiHL(def_RECSIZ);

    // AS
    if (has[2]) {
      // pop hl
      addPopHL();
      // ld a, l
      addLdAL();
    } else {
      // xor a
      addXorA();
    }
    // ld (TEMP), a
    addLdiiA(def_TEMP);

    // FOR
    if (has[1]) {
      // pop af
      addPopAF();
    } else {
      // xor a
      addXorA();
    }
    // ld (TEMP+1), a           ; file mode
    addLdiiA(def_TEMP + 1);

    if (opts->megaROM) {
      // ld hl, 0x0000             ; get return point address
      mark = addMark();
      addLdHLmegarom();
      // ld c, l
      addLdCL();
      // ld b, h
      addLdBH();
    } else {
      // ld bc, 0x0000             ; get return point address
      mark = addMark();
      addLdBC(0x0000);
    }

    // FILE
    if (has[0]) {
      // pop hl
      addPopHL();
    } else {
      // ld hl, NULL STR
      addLdHL(0x8000);
    }

    // push bc            ; save return point to run after FILESPEC
    addPushBC();

    // ld e, (hl)
    addLdEiHL();

    // inc hl
    addIncHL();

    // push hl
    addPushHL();

    // jp FILESPEC        ; in e=string size, hl=string start; out d = device
    // code, FILNAM
    addJp(def_FILESPEC);

    mark->symbol->address = code_pointer;  // return point after FILESPEC

    // ld a, (TEMP+1)
    addLdAii(def_TEMP + 1);
    // ld e, a                ; file mode
    addLdEA();

    // ld a, (TEMP)           ; io number
    addLdAii(def_TEMP);

    // ld hl, fake empty line
    addLdHL(def_ENDPRG);

    // call OPEN     ; in: a = i/o number, e = filemode, d = devicecode
    addCall(def_OPEN);

    addDisableBasicSlot();

  } else {
    syntaxError("Empty OPEN statement");
  }
}

void Compiler::cmd_close() {
  ActionNode* action;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      result_subtype = evalExpression(action);
      addCast(result_subtype, Lexeme::subtype_numeric);

      // ld a, l
      addLdAL();
      // push af
      addPushAF();

      addEnableBasicSlot();

      // pop af
      addPopAF();
      // ld hl, fake empty line
      addLdHL(def_ENDPRG);

      // call CLOSE    ; in: a = i/o number
      addCall(def_CLOSE);

      addDisableBasicSlot();
    }

  } else {
    addEnableBasicSlot();

    // ld hl, fake empty line
    addLdHL(def_ENDPRG);

    // call ClOSE ALL
    addCall(def_CLOSE_ALL);

    addDisableBasicSlot();
  }
}

void Compiler::cmd_def() {
  ActionNode *action, *subaction;
  Lexeme* lexeme;
  unsigned int t = current_action->actions.size();
  int result_subtype;

  if (t) {
    action = current_action->actions[0];
    lexeme = action->lexeme;

    if (lexeme->value == "USR") {
      t = action->actions.size();

      if (t == 2) {
        has_defusr = true;

        subaction = action->actions[0];
        result_subtype = evalExpression(subaction);
        addCast(result_subtype, Lexeme::subtype_numeric);

        // push hl
        addPushHL();

        subaction = action->actions[1];
        result_subtype = evalExpression(subaction);
        addCast(result_subtype, Lexeme::subtype_numeric);

        // ex de,hl
        addExDEHL();

        // pop hl
        addPopHL();
        // ld bc, USRTAB
        addLdBC(def_USRTAB);
        // add hl,hl
        addAddHLHL();
        // add hl,bc
        addAddHLBC();
        // ld (hl), e
        addLdiHLE();
        // inc hl
        addIncHL();
        // ld (hl), d
        addLdiHLD();

      } else {
        syntaxError("Wrong DEF USR parameters count");
      }
    }
  }
}

void Compiler::cmd_cmd() {
  ActionNode *action, *sub_action1, *sub_action2, *sub_action3;
  Lexeme* lexeme;
  unsigned int i, t = current_action->actions.size();
  int result_subtype;

  if (t) {
    for (i = 0; i < t; i++) {
      action = current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->value == "RUNASM") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_runasm
          addCall(def_cmd_runasm);

        } else {
          syntaxError("CMD RUNASM syntax error");
        }

      } else if (lexeme->value == "RUNBAS") {
        syntaxError("CMD RUNBAS not implemented yet");

      } else if (lexeme->value == "WRTVRAM") {
        if (action->actions.size() == 2) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          sub_action2 = action->actions[1];
          result_subtype = evalExpression(sub_action2);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG), hl
          addLdiiHL(def_ARG);

          // call cmd_runasm
          addCall(def_cmd_wrtvram);

        } else {
          syntaxError("CMD WRTVRAM syntax error");
        }

      } else if (lexeme->value == "WRTFNT") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_wrtfnt
          addCall(def_cmd_wrtfnt);

        } else {
          syntaxError("CMD WRTFNT syntax error");
        }

      } else if (lexeme->value == "WRTCHR") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_wrtchr
          addCall(def_cmd_wrtchr);

        } else {
          syntaxError("CMD WRTCHR syntax error");
        }

      } else if (lexeme->value == "WRTCLR") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_wrtclr
          addCall(def_cmd_wrtclr);

        } else {
          syntaxError("CMD WRTCLR syntax error");
        }

      } else if (lexeme->value == "WRTSCR") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_wrtscr
          addCall(def_cmd_wrtscr);

        } else {
          syntaxError("CMD WRTSCR syntax error");
        }

      } else if (lexeme->value == "WRTSPR") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_wrtspr                    ; tiny sprite loader
          addCall(def_cmd_wrtspr);

        } else {
          syntaxError("CMD WRTSPR syntax error");
        }

      } else if (lexeme->value == "WRTSPRPAT") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_wrtsprpat
          addCall(def_cmd_wrtsprpat);

        } else {
          syntaxError("CMD WRTSPRPAT syntax error");
        }

      } else if (lexeme->value == "WRTSPRCLR") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_wrtsprclr
          addCall(def_cmd_wrtsprclr);

        } else {
          syntaxError("CMD WRTSPRCLR syntax error");
        }

      } else if (lexeme->value == "WRTSPRATR") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_wrtspratr
          addCall(def_cmd_wrtspratr);

        } else {
          syntaxError("CMD WRTSPRATR syntax error");
        }

      } else if (lexeme->value == "RAMTOVRAM") {
        if (action->actions.size() == 3) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          sub_action2 = action->actions[1];
          result_subtype = evalExpression(sub_action2);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG), hl
          addLdiiHL(def_ARG);

          sub_action3 = action->actions[2];
          result_subtype = evalExpression(sub_action3);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG2), hl
          addLdiiHL(def_ARG2);

          // call cmd_ramtovram
          addCall(def_cmd_ramtovram);

        } else {
          syntaxError("CMD RAMTOVRAM syntax error");
        }

      } else if (lexeme->value == "VRAMTORAM") {
        if (action->actions.size() == 3) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          sub_action2 = action->actions[1];
          result_subtype = evalExpression(sub_action2);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG), hl
          addLdiiHL(def_ARG);

          sub_action3 = action->actions[2];
          result_subtype = evalExpression(sub_action3);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG2), hl
          addLdiiHL(def_ARG2);

          // call cmd_vramtoram
          addCall(def_cmd_vramtoram);

        } else {
          syntaxError("CMD VRAMTORAM syntax error");
        }

      } else if (lexeme->value == "DISSCR") {
        // call cmd_disscr
        addCall(def_cmd_disscr);

      } else if (lexeme->value == "ENASCR") {
        // call cmd_enascr
        addCall(def_cmd_enascr);

      } else if (lexeme->value == "KEYCLKOFF") {
        // call cmd_keyclkoff
        addCall(def_cmd_keyclkoff);

      } else if (lexeme->value == "MUTE") {
        // call cmd_mute
        addCall(def_cmd_mute);

      } else if (lexeme->value == "PLAY") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          // xor a
          addXorA();
          // ld (ARG), a
          addLdiiA(def_ARG);

          // call cmd_play
          addCall(def_cmd_play);

        } else if (action->actions.size() == 2) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          sub_action2 = action->actions[1];
          result_subtype = evalExpression(sub_action2);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG), hl
          addLdiiHL(def_ARG);

          // call cmd_play
          addCall(def_cmd_play);

        } else {
          syntaxError("CMD PLAY syntax error");
        }

      } else if (lexeme->value == "DRAW") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_draw
          addCall(def_cmd_draw);

        } else {
          syntaxError("CMD DRAW syntax error");
        }

      } else if (lexeme->value == "PT3LOAD") {
        pt3 = true;
        syntaxError("Arkos Tracker not permitted with PT3 player");

      } else if (lexeme->value == "PT3PLAY") {
        pt3 = true;
        syntaxError("Arkos Tracker not permitted with PT3 player");

      } else if (lexeme->value == "PT3MUTE") {
        pt3 = true;
        syntaxError("Arkos Tracker not permitted with PT3 player");

      } else if (lexeme->value == "PT3LOOP") {
        pt3 = true;
        syntaxError("Arkos Tracker not permitted with PT3 player");

      } else if (lexeme->value == "PT3REPLAY") {
        pt3 = true;
        syntaxError("Arkos Tracker not permitted with PT3 player");

      } else if (lexeme->value == "PLYLOAD") {
        akm = true;

        if (action->actions.size()) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          if (action->actions.size() > 1) {
            sub_action2 = action->actions[1];
            result_subtype = evalExpression(sub_action2);
            addCast(result_subtype, Lexeme::subtype_numeric);
          }

          // ld (ARG), hl
          addLdiiHL(def_ARG);

        } else {
          syntaxError("CMD PLYLOAD syntax error");
        }

        // call cmd_plyload
        addCall(def_cmd_plyload);

      } else if (lexeme->value == "PLYSONG") {
        akm = true;

        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

        } else {
          syntaxError("CMD PLYSONG syntax error");
        }

        // call cmd_plysong
        addCall(def_cmd_plysong);

      } else if (lexeme->value == "PLYPLAY") {
        akm = true;

        // call cmd_plyplay
        addCall(def_cmd_plyplay);

      } else if (lexeme->value == "PLYMUTE") {
        akm = true;

        // call cmd_akmmute
        addCall(def_cmd_plymute);

      } else if (lexeme->value == "PLYLOOP") {
        akm = true;

        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_plyloop
          addCall(def_cmd_plyloop);

        } else {
          syntaxError("CMD PLYLOOP syntax error");
        }

      } else if (lexeme->value == "PLYREPLAY") {
        akm = true;

        // call cmd_plyreplay
        addCall(def_cmd_plyreplay);

      } else if (lexeme->value == "PLYSOUND") {
        if (action->actions.size()) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          if (action->actions.size() > 1) {
            sub_action2 = action->actions[1];
            result_subtype = evalExpression(sub_action2);
            addCast(result_subtype, Lexeme::subtype_numeric);
          } else {
            // ld hl, 0
            addLdHL(0x0000);
          }

          // ld (ARG), hl
          addLdiiHL(def_ARG);

          if (action->actions.size() > 2) {
            sub_action2 = action->actions[1];
            result_subtype = evalExpression(sub_action2);
            addCast(result_subtype, Lexeme::subtype_numeric);
          } else if (action->actions.size() > 1) {
            // ld hl, 0
            addLdHL(0x0000);
          }

          // ld (ARG+2), hl
          addLdiiHL(def_ARG + 2);

        } else {
          syntaxError("CMD PLYSOUND syntax error");
        }

        // call cmd_plysound
        addCall(def_cmd_plysound);

      } else if (lexeme->value == "MTF") {
        if (!action->actions.size()) {
          syntaxError("CMD MTF parameters is missing");
        } else if (action->actions.size() > 4) {
          syntaxError("CMD MTF syntax error");
        } else {
          // resource number parameter
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          if (action->actions.size() > 1) {
            addPushHL();

            // map operation parameter
            sub_action2 = action->actions[1];
            result_subtype = evalExpression(sub_action2);
            addCast(result_subtype, Lexeme::subtype_numeric);

            if (action->actions.size() > 2) {
              addLdHL();
              addPushHL();

              // col/x parameter
              sub_action2 = action->actions[2];
              result_subtype = evalExpression(sub_action2);
              addCast(result_subtype, Lexeme::subtype_numeric);

              if (action->actions.size() > 3) {
                addPushHL();

                // row/y parameter
                sub_action2 = action->actions[3];
                result_subtype = evalExpression(sub_action2);
                addCast(result_subtype, Lexeme::subtype_numeric);

                addLdCL();
                addLdBH();   //! row/y
                addPopDE();  //! col/x
                addPopAF();  //! map operation
                addPopHL();  //! resource number
              } else {
                addExDEHL();  //! col/x
                addLdBC(0);   //! row/y
                addPopAF();   //! map operation
                addPopHL();   //! resource number
              }
            } else {
              addXorA();
              addLdEA();
              addLdDA();  //! col/x
              addLdCA();
              addLdBA();   //! row/y
              addLdAL();   //! map operation
              addPopHL();  //! resource number
            }
          } else {
            addXorA();  //! map operation
            addLdEA();
            addLdDA();  //! col/x
            addLdCA();
            addLdBA();  //! row/y
          }

          // hl = resource number
          // de = col/x position
          // bc = row/y position
          // a = map operation
          // call cmd_mtf
          addCall(def_cmd_mtf);
        }
      } else if (lexeme->value == "SETFNT") {
        font = true;

        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          // ld a, 0xff                ; it means all screen banks
          addLdA(0xFF);
          // ld (ARG), a
          addLdiiA(def_ARG);

          // call cmd_setfnt
          addCall(def_cmd_setfnt);

        } else if (action->actions.size() == 2) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          sub_action2 = action->actions[1];
          result_subtype = evalExpression(sub_action2);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG), hl             ; screen font bank number
          addLdiiHL(def_ARG);

          // call cmd_setfnt
          addCall(def_cmd_setfnt);

        } else {
          syntaxError("CMD SETFNT syntax error");
        }
      } else if (lexeme->value == "UPDFNTCLR") {
        // call cmd_disscr
        addCall(def_cmd_updfntclr);
      } else if (lexeme->value == "CLRSCR") {
        // call cmd_clrscr
        addCall(def_cmd_clrscr);
      } else if (lexeme->value == "RAMTORAM") {
        if (action->actions.size() == 3) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          sub_action2 = action->actions[1];
          result_subtype = evalExpression(sub_action2);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG), hl
          addLdiiHL(def_ARG);

          sub_action3 = action->actions[2];
          result_subtype = evalExpression(sub_action3);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (ARG2), hl
          addLdiiHL(def_ARG2);

          // call cmd_ramtoram
          addCall(def_cmd_ramtoram);

        } else {
          syntaxError("CMD RAMTORAM syntax error");
        }
      } else if (lexeme->value == "RSCTORAM") {
        if (action->actions.size() == 2 || action->actions.size() == 3) {
          sub_action1 = action->actions[0];  // resource number
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // push hl
          addPushHL();

          sub_action2 = action->actions[1];  // ram dest address
          result_subtype = evalExpression(sub_action2);
          addCast(result_subtype, Lexeme::subtype_numeric);

          if (action->actions.size() == 2) {
            // ex de, hl
            addExDEHL();
            // xor a   ; no pletter
            addXorA();
          } else {
            // push hl
            addPushHL();

            sub_action3 = action->actions[2];  // pletter? 0=no, 1=yes
            result_subtype = evalExpression(sub_action3);
            addCast(result_subtype, Lexeme::subtype_numeric);

            // ld a, l
            addLdAL();

            // pop de
            addPopDE();
          }

          // pop hl
          addPopHL();

          // call cmd_rsctoram
          addCall(def_cmd_rsctoram);

        } else {
          syntaxError("CMD RSCTORAM syntax error");
        }
      } else if (lexeme->value == "CLRKEY") {
        // call cmd_clrkey
        addCall(def_cmd_clrkey);
      } else if (lexeme->value == "CLIP") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld a, l
          addLdAL();
          // ld hl, 0xfafc
          addLdHL(0xfafc);
          // res 4, (hl)
          addWord(0xCB, 0xA6);
          // bit 0, a
          addWord(0xCB, 0x47);
          // jr z, +3
          addJrZ(0x02);
          //   set 4, (hl)
          addWord(0xCB, 0xE6);

        } else {
          syntaxError("CMD CLIP syntax error");
        }
      } else if (lexeme->value == "TURBO") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);
          // call cmd_turbo
          addCall(def_cmd_turbo);

        } else {
          syntaxError("CMD TURBO syntax error");
        }
      } else if (lexeme->value == "RESTORE") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld (DAC), hl
          addLdiiHL(def_DAC);

          // call cmd_restore
          addCall(def_cmd_restore);  // MSXBAS2ROM resource RESTORE statement

        } else {
          syntaxError("CMD RESTORE syntax error");
        }
      } else if (lexeme->value == "PAGE") {
        if (action->actions.size() == 1) {
          sub_action1 = action->actions[0];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // ld de, 0                 ; delay = stop
          addLdDE(0);

          // push de
          addPushDE();

          // pop bc
          addPopBC();

        } else if (action->actions.size() == 2) {
          sub_action1 = action->actions[1];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // push hl
          addPushHL();
          // push hl
          addPushHL();

          sub_action2 = action->actions[0];
          result_subtype = evalExpression(sub_action2);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // pop de
          addPopDE();

          // pop bc
          addPopBC();

        } else if (action->actions.size() == 3) {
          sub_action1 = action->actions[2];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // push hl
          addPushHL();

          sub_action1 = action->actions[1];
          result_subtype = evalExpression(sub_action1);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // push hl
          addPushHL();

          sub_action2 = action->actions[0];
          result_subtype = evalExpression(sub_action2);
          addCast(result_subtype, Lexeme::subtype_numeric);

          // pop de
          addPopDE();

          // pop bc
          addPopBC();

        } else {
          syntaxError("CMD PAGE syntax error");
        }

        // call cmd_page (l = mode, e = delay #1, c = delay #2)
        addCall(def_cmd_page);
      } else {
        syntaxError("CMD statement invalid");
        return;
      }
    }
  } else {
    syntaxError("CMD with empty parameters");
  }
}

void Compiler::addEnableBasicSlot() {
  if (opts->megaROM) {
    // ld a, (EXPTBL)
    addLdAii(def_EXPTBL);
    // ld h,040h        ; <--- enable jump to here
    addLdH(0x40);
    // call ENASLT		; Select the ROM on page 4000h
    addCall(def_ENASLT);
    // ei
    addEI();
  } else {
    // call enable basic slot function
    if (enable_basic_mark) {
      addFix(enable_basic_mark->symbol);
    } else {
      enable_basic_mark = addMark();
    }
    addCall(0x0000);
  }
}

void Compiler::addDisableBasicSlot() {
  if (opts->megaROM) {
    // ld a, (SLTSTR)
    addLdAii(def_SLTSTR);
    // ld h,040h        ; <--- enable jump to here
    addLdH(0x40);
    // call ENASLT		; Select the ROM on page 4000h
    addCall(def_ENASLT);
    // ei
    addEI();
  } else {
    // call disable basic slot function
    if (disable_basic_mark)
      addFix(disable_basic_mark->symbol);
    else
      disable_basic_mark = addMark();
    addCall(0x0000);
  }
}

void Compiler::addSupportSymbols() {
  // IO REDIRECT FUNCTION
  if (io_redirect_mark) {
    io_redirect_mark->symbol->address = code_pointer;
    // ld a, l  ; io number
    addLdAL();
    // ld hl, fake empty line
    addLdHL(def_ENDPRG);
    // ld ix, IOREDIRECT     ; in: a = i/o number
    addLdIX(def_IOREDIRECT);
    // call CALBAS
    addCall(def_CALBAS);
    // ei
    addEI();
    // ret
    addRet();
  }

  // IO SCREEN FUNCTION
  if (io_screen_mark) {
    io_screen_mark->symbol->address = code_pointer;
    // ld hl, fake empty line
    addLdHL(def_ENDPRG);
    // ld ix, IOTOSCREEN
    addLdIX(def_IOTOSCREEN);
    // call CALBAS
    addCall(def_CALBAS);
    // ei
    addEI();
    // ret
    addRet();
  }

  // DRAW STATEMENT - in: hl (pointer to string)
  if (draw_mark) {
    draw_mark->symbol->address = code_pointer;
    // ld a, (SCRMOD)
    addLdAii(def_SCRMOD);
    // cp 2
    addCp(0x02);
    // ret c                    ; return if text mode
    addRetC();

    // push hl
    addPushHL();

    addEnableBasicSlot();

    // pop hl
    addPopHL();

    // xor a
    addXorA();
    // ld bc, disable basic slot
    if (disable_basic_mark)
      addFix(disable_basic_mark->symbol);
    else
      disable_basic_mark = addMark();
    addLdBC(0x0000);
    // push bc
    addPushBC();
    // ld bc, 0
    addLdBC(0x0000);
    // push bc
    addPushBC();
    // push bc
    addPushBC();
    // push bc
    addPushBC();
    // ld de, 0x5D83
    addLdDE(0x5D83);
    // ld (0xFCBB), a   ; DRWFLG
    addLdiiA(0xFCBB);
    // ld (0xF958), a   ; MCLFLG
    addLdiiA(0xF958);
    // ld (0xF956), de  ; MCLTAB
    addLdiiDE(0xF956);
    // ld a, (hl)
    addLdAiHL();
    // inc hl
    addIncHL();
    // ld (0xFB3B), a   ; MCLLEN
    addLdiiA(0xFB3B);
    // jp 0x5691    ; DRAW subroutine = 0x568C+5  (main routine hook address =
    // 0x39A8)
    addJp(0x5691);
  }

  // ENABLE BASIC SLOT FUNCTION
  if (enable_basic_mark) {
    enable_basic_mark->symbol->address = code_pointer;
    // ld a, (EXPTBL)
    addLdAii(def_EXPTBL);
    // jr $+4            ; jump to disable code
    addJr(0x03);
  }

  // DISABLE BASIC SLOT FUNCTION
  if (disable_basic_mark) {
    disable_basic_mark->symbol->address = code_pointer;
    // ld a, (SLTSTR)
    addLdAii(def_SLTSTR);
    // ld h,040h        ; <--- enable jump to here
    addLdH(0x40);
    // call ENASLT		; Select the ROM on page 4000h
    addCall(def_ENASLT);
    // ei
    addEI();
    // ret
    addRet();
  }
}

//-------------------------------------------------------------------------------------------

void Compiler::syntaxError() {
  syntaxError("Syntax error");
}

void Compiler::syntaxError(string msg) {
  compiled = false;
  error_message = msg;
  if (current_tag) error_message += " (line=" + current_tag->value + ")";
}
