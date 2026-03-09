#include "compiler_screen_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"

void CompilerScreenStatementStrategy::cmd_screen(CompilerContext* context) {
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
      cmd_screen_copy(context);
      return;
    } else if (lexeme->value == "PASTE") {
      context->current_action = action;
      cmd_screen_paste(context);
      return;
    } else if (lexeme->value == "SCROLL") {
      context->current_action = action;
      cmd_screen_scroll(context);
      return;
    } else if (lexeme->value == "LOAD") {
      context->current_action = action;
      cmd_screen_load(context);
      return;
    } else if (lexeme->value == "ON") {
      context->current_action = action;
      cmd_screen_on(context);
      return;
    } else if (lexeme->value == "OFF") {
      context->current_action = action;
      cmd_screen_off(context);
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

void CompilerScreenStatementStrategy::cmd_screen_copy(
    CompilerContext* context) {
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

void CompilerScreenStatementStrategy::cmd_screen_paste(
    CompilerContext* context) {
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

void CompilerScreenStatementStrategy::cmd_screen_scroll(
    CompilerContext* context) {
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

void CompilerScreenStatementStrategy::cmd_screen_load(
    CompilerContext* context) {
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

void CompilerScreenStatementStrategy::cmd_screen_on(CompilerContext* context) {
  auto& cpu = *context->cpu;
  // call cmd_enascr
  cpu.addCall(def_cmd_enascr);
}

void CompilerScreenStatementStrategy::cmd_screen_off(CompilerContext* context) {
  auto& cpu = *context->cpu;
  // call cmd_disscr
  cpu.addCall(def_cmd_disscr);
}

bool CompilerScreenStatementStrategy::execute(CompilerContext* context) {
  cmd_screen(context);
  return context->compiled;
}
