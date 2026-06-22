#include "compiler_set_statement_strategy.h"

#include "action_node.h"
#include "compiler_code_optimizer.h"
#include "compiler_cls_statement_strategy.h"
#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "fix_node.h"
#include "lexeme.h"

void CompilerSetStatementStrategy::cmd_set(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  shared_ptr<ActionNode> action;
  shared_ptr<Lexeme> next_lexeme;
  unsigned int t = context->current_action->actions.size();
  shared_ptr<FixNode> skipMsx1Mark;

  if (t == 1) {
    action = context->current_action->actions[0];
    next_lexeme = action->lexeme;

    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "TILE") {
        cmd_set_tile(context);
        return;
      } else if (next_lexeme->value == "SPRITE") {
        cmd_set_sprite(context);
        return;
      } else if (next_lexeme->value == "FONT") {
        cmd_set_font(context);
        return;
      }
    }

    // ld a, (BIOS VERSION)
    cpu.addLdAii(def_VERSION);
    // and a
    cpu.addAndA();
    // jp z, $                ; skip if MSX1
    skipMsx1Mark = fixup.addMark();
    cpu.addJpZ(0x0000);

    if (next_lexeme->type == Lexeme::type_keyword &&
        next_lexeme->value == "ADJUST") {
      cmd_set_adjust(context);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "BEEP") {
      cmd_set_beep(context);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "DATE") {
      cmd_set_date(context);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "PAGE") {
      cmd_set_page(context);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "PASSWORD") {
      context->syntaxError(
          "SET PASSWORD will not be supported for end-user security");
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "PROMPT") {
      cmd_set_prompt(context);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "SCREEN") {
      cmd_set_screen(context);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "SCROLL") {
      cmd_set_scroll(context);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "TIME") {
      cmd_set_time(context);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "TITLE") {
      cmd_set_title(context);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "VIDEO") {
      cmd_set_video(context);
    } else {
      context->syntaxError("Invalid SET statement");
    }

    skipMsx1Mark->aimHere();

  } else {
    context->syntaxError("Wrong SET parameters count");
  }
}

void CompilerSetStatementStrategy::cmd_set_video(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& codeHelper = *context->codeHelper;
  shared_ptr<ActionNode> action = context->current_action->actions[0];
  shared_ptr<ActionNode> sub_action;
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

void CompilerSetStatementStrategy::cmd_set_adjust(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& codeHelper = *context->codeHelper;
  shared_ptr<ActionNode> action = context->current_action->actions[0];
  shared_ptr<ActionNode> sub_action;
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

void CompilerSetStatementStrategy::cmd_set_screen(
    shared_ptr<CompilerContext> context) {
  auto& codeHelper = *context->codeHelper;
  shared_ptr<ActionNode> action = context->current_action->actions[0];
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

void CompilerSetStatementStrategy::cmd_set_beep(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& codeHelper = *context->codeHelper;
  shared_ptr<ActionNode> action = context->current_action->actions[0];
  shared_ptr<ActionNode> sub_action;
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

void CompilerSetStatementStrategy::cmd_set_title(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& codeHelper = *context->codeHelper;
  shared_ptr<ActionNode> action = context->current_action->actions[0];
  shared_ptr<ActionNode> sub_action;
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

void CompilerSetStatementStrategy::cmd_set_prompt(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& codeHelper = *context->codeHelper;
  shared_ptr<ActionNode> action = context->current_action->actions[0];
  shared_ptr<ActionNode> sub_action;
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

void CompilerSetStatementStrategy::cmd_set_page(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& optimizer = *context->codeOptimizer;
  shared_ptr<ActionNode> action = context->current_action->actions[0];
  shared_ptr<ActionNode> sub_action;
  shared_ptr<Lexeme> lexeme;
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
      optimizer.addKernelCall(DISP_XBASIC_SET_PAGE);
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

void CompilerSetStatementStrategy::cmd_set_scroll(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& optimizer = *context->codeOptimizer;
  shared_ptr<ActionNode> action = context->current_action->actions[0];
  shared_ptr<ActionNode> sub_action;
  shared_ptr<Lexeme> lexeme;
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
    optimizer.addKernelCall(DISP_XBASIC_SET_SCROLL);

  } else {
    context->syntaxError("Wrong parameters count on SET SCROLL statement");
  }
}

void CompilerSetStatementStrategy::cmd_set_tile(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<ActionNode> action = context->current_action->actions[0];
  shared_ptr<ActionNode> sub_action, sub_sub_action;
  shared_ptr<Lexeme> lexeme, sub_lexeme;
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
      context->codeOptimizer->addKernelCall(DISP_XBASIC_LOCATE);
      // ld a, 0xff                ; it means all screen banks
      cpu.addLdA(0xFF);
      // ld (ARG), a
      cpu.addLdiiA(def_ARG);
      // call cmd_setfnt
      context->codeOptimizer->addKernelCall(DISP_cmd_setfnt);

    } else if (lexeme->value == "OFF") {
      // ld a, 1      ; graphical mode
      cpu.addLdA(1);
      // ld (SOMODE), a
      cpu.addLdiiA(def_SOMODE);

    } else if (lexeme->value == "FLIP") {
      if (t >= 2 && t <= 3) {
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

        // bank number (optional)
        if (t == 3) {
          // save direction before evaluating bank (clobbers HL)
          cpu.addPushHL();
          sub_action = action->actions[2];
          result_subtype = expression.evalExpression(sub_action);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);
          // ld b, l
          cpu.addLdBL();
          // restore direction to HL
          cpu.addPopHL();
        } else {
          // ld b, 3     ; default = all banks
          cpu.addLdB(0x03);
        }

        context->codeOptimizer->addKernelCall(DISP_set_tile_flip);

      } else {
        context->syntaxError(
            "Wrong parameters count on SET TILE FLIP statement");
      }

    } else if (lexeme->value == "ROTATE") {
      if (t >= 2 && t <= 3) {
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

        // bank number (optional)
        if (t == 3) {
          // save direction before evaluating bank (clobbers HL)
          cpu.addPushHL();
          sub_action = action->actions[2];
          result_subtype = expression.evalExpression(sub_action);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);
          // ld b, l
          cpu.addLdBL();
          // restore direction to HL
          cpu.addPopHL();
        } else {
          // ld b, 3     ; default = all banks
          cpu.addLdB(0x03);
        }

        context->codeOptimizer->addKernelCall(DISP_set_tile_rotate);

      } else {
        context->syntaxError(
            "Wrong parameters count on SET TILE ROTATE statement");
      }

    } else if (lexeme->value == "PATTERN") {
      if (t >= 2) {
        sub_action = action->actions[1];
        lexeme = sub_action->lexeme;

        bool isBuffer = false;

        if (lexeme->value == "ARRAY") {
          if (sub_action->actions.size() == 1 &&
              sub_action->actions[0]->lexeme->type == Lexeme::type_identifier) {
            // array variable with (): PB%()
            sub_action = sub_action->actions[0];
            isBuffer = true;
          }
        } else if (lexeme->type == Lexeme::type_identifier) {
          // plain identifier without parens: PB%
          isBuffer = true;
        }

        if (isBuffer) {
          // buffer form: <4-int-array> [, <bank>]
          if (t == 2 || t == 3) {
          // tile number
          result_subtype = expression.evalExpression(action->actions[0]);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);
          cpu.addLdAL();
          cpu.addPushAF();

          // buffer address
          sub_lexeme = sub_action->lexeme;
          if (sub_lexeme->type == Lexeme::type_identifier) {
            fixup.addFix(sub_lexeme);
            cpu.addLdHL(0x0000);
            result_subtype = Lexeme::subtype_numeric;
          } else {
            result_subtype = expression.evalExpression(sub_action);
          }
          expression.addCast(result_subtype, Lexeme::subtype_numeric);

          if (t == 3) {
            // push hl (buffer)
            cpu.addPushHL();
            // bank number
            result_subtype = expression.evalExpression(action->actions[2]);
            expression.addCast(result_subtype, Lexeme::subtype_numeric);
            // ld b, l
            cpu.addLdBL();
            // pop hl (buffer)
            cpu.addPopHL();
            // pop af (tile number)
            cpu.addPopAF();
          } else {
            // ld b, 3     ; default = all banks
            cpu.addLdB(0x03);
            // pop af (tile number)
            cpu.addPopAF();
          }

          // call set_tile_pattern_buffer
          //   a = tile number
          //   hl = pointer to an 8 bytes buffer
          //   b = bank (0-2 specific, 3=all)
          context->codeOptimizer->addKernelCall(DISP_set_tile_pattern_buffer);
          } else {
          context->syntaxError(
              "Wrong parameters count on SET TILE PATTERN statement");
          }

        } else if (lexeme->value == "ARRAY") {
          // inline tuple form: (<l0>,...,<l7>) [, <bank>]
          if (t <= 3) {
          // tile number
          result_subtype = expression.evalExpression(action->actions[0]);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);
          // ld (ARG), hl
          cpu.addLdiiHL(def_ARG);

          // bank number
          if (t == 3) {
            result_subtype = expression.evalExpression(action->actions[2]);
            expression.addCast(result_subtype, Lexeme::subtype_numeric);
            // ld h, l
            cpu.addLdHL();
          } else {
            // ld h, 0x03
            cpu.addLdH(0x03);
          }
          // ld (ARG2), hl
          cpu.addLdiiHL(def_ARG2);

          // pattern data (always ARRAY at this point)
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
              context->codeOptimizer->addKernelCall(DISP_set_tile_pattern);
            }
          }
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
      if (t < 2) {
        context->syntaxError(
            "Wrong parameters count on SET TILE COLOR statement");

      } else {
        sub_action = action->actions[1];
        sub_lexeme = sub_action->lexeme;

        // Buffer form: actions[1] is a variable/array identifier
        if ((sub_lexeme->type == Lexeme::type_identifier) ||
            (sub_lexeme->value == "ARRAY" && sub_action->actions.size() == 1 &&
             sub_action->actions[0]->lexeme->type == Lexeme::type_identifier)) {
          if (t == 2 || t == 3) {
            // tile number
            sub_action = action->actions[0];
            result_subtype = expression.evalExpression(sub_action);
            expression.addCast(result_subtype, Lexeme::subtype_numeric);
            cpu.addLdAL();
            cpu.addPushAF();

            // tile buffer pointer (8 bytes)
            sub_action = action->actions[1];
            sub_lexeme = sub_action->lexeme;
            if (sub_lexeme->type == Lexeme::type_identifier) {
              fixup.addFix(sub_lexeme);
              cpu.addLdHL(0x0000);
              result_subtype = Lexeme::subtype_numeric;
            } else {
              sub_sub_action = sub_action->actions[0];
              fixup.addFix(sub_sub_action->lexeme);
              cpu.addLdHL(0x0000);
              result_subtype = Lexeme::subtype_numeric;
            }
            expression.addCast(result_subtype, Lexeme::subtype_numeric);

            if (t == 3) {
              cpu.addPushHL();
              sub_action = action->actions[2];
              result_subtype = expression.evalExpression(sub_action);
              expression.addCast(result_subtype, Lexeme::subtype_numeric);
              cpu.addLdBL();
              cpu.addPopHL();
              cpu.addPopAF();
            } else {
              cpu.addLdB(0x03);
              cpu.addPopAF();
            }

            context->codeOptimizer->addKernelCall(DISP_set_tile_color_buf);

          } else {
            context->syntaxError(
                "Wrong parameters count on SET TILE COLOR statement");
          }

        } else if (t >= 2 && t <= 4) {
        // tile number
        sub_action = action->actions[0];
        // ld hl, parameter value    ; tile number
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        // ld (ARG), hl
        cpu.addLdiiHL(def_ARG);

        // color data
        sub_action = action->actions[1];
        lexeme = sub_action->lexeme;

        // bank number
        if (t == 4) {
          sub_action = action->actions[3];
          result_subtype = expression.evalExpression(sub_action);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);
          cpu.addLdHL();
        } else if (t == 3 && lexeme->value == "ARRAY") {
          sub_lexeme = action->actions[2]->lexeme;
          if (sub_lexeme->value != "ARRAY" &&
              sub_lexeme->type != Lexeme::type_identifier) {
            sub_action = action->actions[2];
            result_subtype = expression.evalExpression(sub_action);
            expression.addCast(result_subtype, Lexeme::subtype_numeric);
            cpu.addLdHL();
          } else {
            cpu.addLdH(0x03);
          }
        } else {
          cpu.addLdH(0x03);
        }
        cpu.addLdiiHL(def_ARG2);

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
                sub_sub_action = action->actions[2];
                lexeme = sub_sub_action->lexeme;
                if (lexeme->value == "ARRAY") {
                  if (i < sub_sub_action->actions.size()) {
                    sub_sub_action = sub_sub_action->actions[i];
                    lexeme = sub_sub_action->lexeme;
                    if (!(lexeme->type == Lexeme::type_literal &&
                          lexeme->subtype == Lexeme::subtype_null)) {
                      cpu.addPushAF();
                      result_subtype = expression.evalExpression(sub_sub_action);
                      expression.addCast(result_subtype, Lexeme::subtype_numeric);
                      cpu.addPopAF();
                      cpu.addOrL();
                    }
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
              context->codeOptimizer->addKernelCall(DISP_set_tile_color);
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
          context->codeOptimizer->addKernelCall(DISP_set_tile_color);
        }

      } else {
        context->syntaxError(
            "Wrong parameters count on SET TILE COLOR statement");
      }
      }

    } else {
      context->syntaxError("Invalid syntax on SET TILE statement");
    }

  } else {
    context->syntaxError("Missing parameters on SET TILE statement");
  }
}

void CompilerSetStatementStrategy::cmd_set_font(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<ActionNode> action = context->current_action->actions[0];
  shared_ptr<ActionNode> sub_action1, sub_action2;
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
      context->codeOptimizer->addKernelCall(DISP_cmd_setfnt);

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
      context->codeOptimizer->addKernelCall(DISP_cmd_setfnt);

    } else {
      context->syntaxError("Wrong number of parameters on SET FONT");
    }

  } else {
    context->syntaxError("SET FONT syntax error");
  }
}

void CompilerSetStatementStrategy::cmd_set_sprite(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& fixup = *context->fixupResolver;
  shared_ptr<ActionNode> action = context->current_action->actions[0];
  shared_ptr<ActionNode> sub_action;
  shared_ptr<Lexeme> lexeme, sub_lexeme;
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

        context->codeOptimizer->addKernelCall(DISP_set_sprite_flip);

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

        context->codeOptimizer->addKernelCall(DISP_set_sprite_rotate);

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
        context->codeOptimizer->addKernelCall(DISP_set_sprite_pattern);

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
        context->codeOptimizer->addKernelCall(DISP_set_sprite_color);

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

void CompilerSetStatementStrategy::cmd_set_date(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<Lexeme> lexeme;
  shared_ptr<ActionNode> action = context->current_action->actions[0];
  shared_ptr<ActionNode> sub_action;
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

    context->codeOptimizer->addKernelCall(DISP_set_date);

  } else {
    context->syntaxError(
        "Wrong SET DATE parameters count.\nTry: SET DATE iYear, iMonth, "
        "iDay");
  }
}

void CompilerSetStatementStrategy::cmd_set_time(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<Lexeme> lexeme;
  shared_ptr<ActionNode> action = context->current_action->actions[0];
  shared_ptr<ActionNode> sub_action;
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

    context->codeOptimizer->addKernelCall(DISP_set_time);

  } else {
    context->syntaxError(
        "Wrong SET TIME parameters count.\nTry: SET TIME iHour, iMinute, "
        "iSecond");
  }
}

bool CompilerSetStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_set(context);
  return context->compiled;
}
