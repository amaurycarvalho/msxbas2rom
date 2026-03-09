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
