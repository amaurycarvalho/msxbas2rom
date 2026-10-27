#include "compiler_set_statement_strategy.h"

#include "action_node.h"
#include "compiler_code_optimizer.h"
#include "compiler_cls_statement_strategy.h"
#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "compiler_tile_parameter.h"
#include "fix_node.h"
#include "lexeme.h"

void CompilerSetStatementStrategy::cmd_set(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->getCpu();
  auto& fixup = *context->getFixupResolver();
  shared_ptr<ActionNode> action;
  shared_ptr<Lexeme> next_lexeme;
  unsigned int t = context->getCurrentAction()->actions.size();
  shared_ptr<FixNode> skipMsx1Mark;

  if (t == 1) {
    action = context->getCurrentAction()->actions[0];
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
  auto& cpu = *context->getCpu();
  auto& expression = *context->getExpressionEvaluator();
  auto& codeHelper = *context->getCodeHelper();
  shared_ptr<ActionNode> action = context->getCurrentAction()->actions[0];
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
        cpu.context->getCodePointer()--;
        cpu.context->getCodeSize()--;
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
  auto& cpu = *context->getCpu();
  auto& expression = *context->getExpressionEvaluator();
  auto& codeHelper = *context->getCodeHelper();
  shared_ptr<ActionNode> action = context->getCurrentAction()->actions[0];
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
  auto& codeHelper = *context->getCodeHelper();
  shared_ptr<ActionNode> action = context->getCurrentAction()->actions[0];
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
  auto& cpu = *context->getCpu();
  auto& expression = *context->getExpressionEvaluator();
  auto& codeHelper = *context->getCodeHelper();
  shared_ptr<ActionNode> action = context->getCurrentAction()->actions[0];
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
  auto& cpu = *context->getCpu();
  auto& expression = *context->getExpressionEvaluator();
  auto& codeHelper = *context->getCodeHelper();
  shared_ptr<ActionNode> action = context->getCurrentAction()->actions[0];
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
  auto& cpu = *context->getCpu();
  auto& expression = *context->getExpressionEvaluator();
  auto& codeHelper = *context->getCodeHelper();
  shared_ptr<ActionNode> action = context->getCurrentAction()->actions[0];
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
  auto& cpu = *context->getCpu();
  auto& expression = *context->getExpressionEvaluator();
  auto& optimizer = *context->getCodeOptimizer();
  shared_ptr<ActionNode> action = context->getCurrentAction()->actions[0];
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
  auto& cpu = *context->getCpu();
  auto& expression = *context->getExpressionEvaluator();
  auto& optimizer = *context->getCodeOptimizer();
  shared_ptr<ActionNode> action = context->getCurrentAction()->actions[0];
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
  auto& cpu = *context->getCpu();
  auto& fixup = *context->getFixupResolver();
  auto& expression = *context->getExpressionEvaluator();
  shared_ptr<ActionNode> action = context->getCurrentAction()->actions[0];
  shared_ptr<ActionNode> sub_action, sub_sub_action;
  shared_ptr<Lexeme> lexeme, sub_lexeme;
  unsigned int i, t, tt;
  int result_subtype;

  auto emitBank = [&](shared_ptr<ActionNode> bank_action, bool high_byte) {
    bool is_literal = false;
    int literal = normalizedTileBankLiteral(bank_action, is_literal);
    if (is_literal) {
      if (high_byte)
        cpu.addLdH(literal);
      else
        cpu.addLdB(literal);
    } else {
      result_subtype = expression.evalExpression(bank_action);
      expression.addCast(result_subtype, Lexeme::subtype_numeric);
      if (high_byte)
        cpu.addLdHL();
      else
        cpu.addLdBL();
    }
  };

  auto emitColorByte = [&](shared_ptr<ActionNode> fc_action,
                           shared_ptr<ActionNode> bc_action) {
    result_subtype = expression.evalExpression(fc_action);
    expression.addCast(result_subtype, Lexeme::subtype_numeric);
    cpu.addLdAL();
    cpu.addRLA();
    cpu.addRLA();
    cpu.addRLA();
    cpu.addRLA();
    cpu.addAnd(0xF0);

    if (bc_action) {
      cpu.addPushAF();
      result_subtype = expression.evalExpression(bc_action);
      expression.addCast(result_subtype, Lexeme::subtype_numeric);
      cpu.addLdAL();
      cpu.addAnd(0x0F);
      cpu.addLdLA();
      cpu.addPopAF();
      cpu.addOrL();
    }
  };

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
      context->getCodeOptimizer()->addKernelCall(DISP_XBASIC_LOCATE);
      // ld a, 0xff                ; it means all screen banks
      cpu.addLdA(0xFF);
      // ld (ARG), a
      cpu.addLdiiA(def_ARG);
      // call cmd_setfnt
      context->getCodeOptimizer()->addKernelCall(DISP_cmd_setfnt);

    } else if (lexeme->value == "OFF") {
      // ld a, 1      ; graphical mode
      cpu.addLdA(1);
      // ld (SOMODE), a
      cpu.addLdiiA(def_SOMODE);

    } else if (lexeme->value == "FLIP") {
      if (t >= 2 && t <= 3) {
        // tile number
        sub_action = action->actions[0];
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        cpu.addPushHL();

        // direction
        sub_action = action->actions[1];
        int direction = 0;
        if (getNumericLiteral(sub_action, direction) &&
            (direction < 0 || direction > 2)) {
          context->syntaxError("Invalid direction on SET TILE FLIP statement");
          return;
        }
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        cpu.addPopDE();

        // bank number (optional)
        if (t == 3) {
          if (isTileArrayAction(context, action->actions[2])) {
            context->syntaxError(
                "Invalid bank parameter on SET TILE FLIP statement");
            return;
          }
          cpu.addPushHL();
          emitBank(action->actions[2], false);
          cpu.addPopHL();
        } else {
          cpu.addLdB(0x03);
        }

        context->getCodeOptimizer()->addKernelCall(DISP_set_tile_flip);

      } else {
        context->syntaxError(
            "Wrong parameters count on SET TILE FLIP statement");
      }

    } else if (lexeme->value == "ROTATE") {
      if (t >= 2 && t <= 3) {
        // tile number
        sub_action = action->actions[0];
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        cpu.addPushHL();

        // direction
        sub_action = action->actions[1];
        int direction = 0;
        if (getNumericLiteral(sub_action, direction) &&
            (direction < 0 || direction > 2)) {
          context->syntaxError(
              "Invalid direction on SET TILE ROTATE statement");
          return;
        }
        result_subtype = expression.evalExpression(sub_action);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
        cpu.addPopDE();

        // bank number (optional)
        if (t == 3) {
          if (isTileArrayAction(context, action->actions[2])) {
            context->syntaxError(
                "Invalid bank parameter on SET TILE ROTATE statement");
            return;
          }
          cpu.addPushHL();
          emitBank(action->actions[2], false);
          cpu.addPopHL();
        } else {
          cpu.addLdB(0x03);
        }

        context->getCodeOptimizer()->addKernelCall(DISP_set_tile_rotate);

      } else {
        context->syntaxError(
            "Wrong parameters count on SET TILE ROTATE statement");
      }

    } else if (lexeme->value == "PATTERN") {
      if (t >= 2 && t <= 3) {
        shared_ptr<ActionNode> pattern_action = action->actions[1];
        shared_ptr<ActionNode> buffer_action =
            getDeclaredArrayIdentifier(context, pattern_action);
        shared_ptr<ActionNode> bank_action =
            (t == 3) ? action->actions[2] : nullptr;

        if (bank_action && isTileArrayAction(context, bank_action)) {
          context->syntaxError(
              "Invalid bank parameter on SET TILE PATTERN statement");
          return;
        }

        if (buffer_action) {
          // buffer form: <4-int-array> [, <bank>]
          result_subtype = expression.evalExpression(action->actions[0]);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);
          cpu.addLdAL();
          cpu.addPushAF();

          fixup.addFix(buffer_action->lexeme);
          cpu.addLdHL(0x0000);

          if (t == 3) {
            cpu.addPushHL();
            emitBank(bank_action, false);
            cpu.addPopHL();
            cpu.addPopAF();
          } else {
            cpu.addLdB(0x03);
            cpu.addPopAF();
          }

          context->getCodeOptimizer()->addKernelCall(
              DISP_set_tile_pattern_buffer);

        } else if (isTileTupleAction(pattern_action)) {
          // inline tuple form: (<l0>,...,<l7>) [, <bank>]
          tt = pattern_action->actions.size();
          if (tt == 0 || tt > 8) {
            context->syntaxError(
                "Wrong parameters count on SET TILE PATTERN statement");
            return;
          }

          result_subtype = expression.evalExpression(action->actions[0]);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);
          cpu.addLdiiHL(def_ARG);

          if (t == 3)
            emitBank(bank_action, true);
          else
            cpu.addLdH(0x03);
          cpu.addLdiiHL(def_ARG2);

          for (i = 0; i < tt; i++) {
            sub_sub_action = pattern_action->actions[i];
            lexeme = sub_sub_action->lexeme;

            if (lexeme->type == Lexeme::type_literal &&
                lexeme->subtype == Lexeme::subtype_null) {
              continue;
            }

            result_subtype = expression.evalExpression(sub_sub_action);
            expression.addCast(result_subtype, Lexeme::subtype_numeric);

            cpu.addLdBCii(def_ARG2);
            cpu.addLdCL();
            cpu.addLdDE(i);
            cpu.addLdHLii(def_ARG);

            context->getCodeOptimizer()->addKernelCall(DISP_set_tile_pattern);
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
      if (t < 2 || t > 4) {
        context->syntaxError(
            "Wrong parameters count on SET TILE COLOR statement");
      } else {
        shared_ptr<ActionNode> color_action = action->actions[1];
        shared_ptr<ActionNode> buffer_action =
            getDeclaredArrayIdentifier(context, color_action);
        bool color_is_tuple = !buffer_action && isTileTupleAction(color_action);
        bool color_is_scalar = !buffer_action && !color_is_tuple;
        shared_ptr<ActionNode> bank_action = nullptr;
        shared_ptr<ActionNode> bc_action = nullptr;
        bool has_bc = false;

        if (t == 3) {
          if (buffer_action) {
            bank_action = action->actions[2];
          } else if (color_is_tuple) {
            if (isTileTupleAction(action->actions[2])) {
              bc_action = action->actions[2];
              has_bc = true;
            } else if (isTileBufferAction(context, action->actions[2])) {
              context->syntaxError(
                  "Syntax not supported on SET TILE COLOR statement");
              return;
            } else {
              bank_action = action->actions[2];
            }
          } else {
            if (isTileArrayAction(context, action->actions[2])) {
              context->syntaxError(
                  "Syntax not supported on SET TILE COLOR statement");
              return;
            }
            bc_action = action->actions[2];
            has_bc = true;
          }
        } else if (t == 4) {
          if (color_is_tuple) {
            if (!isTileTupleAction(action->actions[2])) {
              context->syntaxError(
                  "Syntax not supported on SET TILE COLOR statement");
              return;
            }
            bc_action = action->actions[2];
            has_bc = true;
          } else if (color_is_scalar) {
            if (isTileArrayAction(context, action->actions[2])) {
              context->syntaxError(
                  "Syntax not supported on SET TILE COLOR statement");
              return;
            }
            bc_action = action->actions[2];
            has_bc = true;
          } else {
            context->syntaxError(
                "Wrong parameters count on SET TILE COLOR statement");
            return;
          }
          bank_action = action->actions[3];
        }

        if (bank_action && isTileArrayAction(context, bank_action)) {
          context->syntaxError(
              "Invalid bank parameter on SET TILE COLOR statement");
          return;
        }

        if (buffer_action) {
          result_subtype = expression.evalExpression(action->actions[0]);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);
          cpu.addLdAL();
          cpu.addPushAF();

          fixup.addFix(buffer_action->lexeme);
          cpu.addLdHL(0x0000);

          if (t == 3) {
            cpu.addPushHL();
            emitBank(bank_action, false);
            cpu.addPopHL();
            cpu.addPopAF();
          } else {
            cpu.addLdB(0x03);
            cpu.addPopAF();
          }

          context->getCodeOptimizer()->addKernelCall(DISP_set_tile_color_buf);
        } else {
          result_subtype = expression.evalExpression(action->actions[0]);
          expression.addCast(result_subtype, Lexeme::subtype_numeric);
          cpu.addLdiiHL(def_ARG);

          if (bank_action)
            emitBank(bank_action, true);
          else
            cpu.addLdH(0x03);
          cpu.addLdiiHL(def_ARG2);

          if (color_is_tuple) {
            tt = color_action->actions.size();
            if (tt == 0 || tt > 8) {
              context->syntaxError(
                  "Wrong parameters count on SET TILE COLOR statement");
              return;
            }
            if (has_bc && bc_action->actions.size() > 8) {
              context->syntaxError(
                  "Wrong parameters count on SET TILE COLOR statement");
              return;
            }

            for (i = 0; i < tt; i++) {
              sub_sub_action = color_action->actions[i];
              lexeme = sub_sub_action->lexeme;

              if (lexeme->type == Lexeme::type_literal &&
                  lexeme->subtype == Lexeme::subtype_null) {
                continue;
              }

              shared_ptr<ActionNode> bc_item = nullptr;
              if (has_bc && i < bc_action->actions.size()) {
                bc_item = bc_action->actions[i];
                shared_ptr<Lexeme> bc_lexeme = bc_item->lexeme;
                if (bc_lexeme->type == Lexeme::type_literal &&
                    bc_lexeme->subtype == Lexeme::subtype_null) {
                  bc_item = nullptr;
                }
              }

              emitColorByte(sub_sub_action, bc_item);

              cpu.addLdBCii(def_ARG2);
              cpu.addLdCA();
              cpu.addLdDE(i);
              cpu.addLdHLii(def_ARG);

              context->getCodeOptimizer()->addKernelCall(DISP_set_tile_color);
            }
          } else {
            emitColorByte(color_action, has_bc ? bc_action : nullptr);

            cpu.addLdBCii(def_ARG2);
            cpu.addLdCA();
            cpu.addLdDE(0x000F);
            cpu.addLdHLii(def_ARG);

            context->getCodeOptimizer()->addKernelCall(DISP_set_tile_color);
          }
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
  auto& cpu = *context->getCpu();
  auto& expression = *context->getExpressionEvaluator();
  shared_ptr<ActionNode> action = context->getCurrentAction()->actions[0];
  shared_ptr<ActionNode> sub_action1, sub_action2;
  unsigned int t;
  int result_subtype;

  t = action->actions.size();
  if (t) {
    context->getFont() = true;

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
      context->getCodeOptimizer()->addKernelCall(DISP_cmd_setfnt);

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
      context->getCodeOptimizer()->addKernelCall(DISP_cmd_setfnt);

    } else {
      context->syntaxError("Wrong number of parameters on SET FONT");
    }

  } else {
    context->syntaxError("SET FONT syntax error");
  }
}

void CompilerSetStatementStrategy::cmd_set_sprite(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->getCpu();
  auto& expression = *context->getExpressionEvaluator();
  auto& fixup = *context->getFixupResolver();
  shared_ptr<ActionNode> action = context->getCurrentAction()->actions[0];
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

        context->getCodeOptimizer()->addKernelCall(DISP_set_sprite_flip);

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

        context->getCodeOptimizer()->addKernelCall(DISP_set_sprite_rotate);

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
        context->getCodeOptimizer()->addKernelCall(DISP_set_sprite_pattern);

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
        context->getCodeOptimizer()->addKernelCall(DISP_set_sprite_color);

      } else {
        context->syntaxError(
            "Wrong parameters count on SET SPRITE COLOR statement");
      }

    } else if (lexeme->value == "HITBOX") {
      cmd_set_sprite_hitbox(context);

    } else {
      context->syntaxError("Invalid syntax on SET SPRITE statement");
    }
  } else {
    context->syntaxError("Missing parameters on SET SPRITE statement");
  }
}

void CompilerSetStatementStrategy::cmd_set_sprite_hitbox(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->getCpu();
  auto& expression = *context->getExpressionEvaluator();
  shared_ptr<ActionNode> spriteAction = context->getCurrentAction()->actions[0];
  shared_ptr<ActionNode> action = spriteAction->actions[0];
  shared_ptr<Lexeme> lexeme;
  unsigned int i, t = action->actions.size();
  int result_subtype, mode = 3;

  if (t < 1 || t > 5) {
    context->syntaxError(
        "Wrong parameters count on SET SPRITE HITBOX statement");
    return;
  }

  lexeme = action->actions[0]->lexeme;

  if (lexeme && lexeme->type == Lexeme::type_keyword &&
      (lexeme->value == "ON" || lexeme->value == "OFF" ||
       lexeme->value == "AUTO")) {
    if (t != 1) {
      context->syntaxError(
          "Wrong parameters count on SET SPRITE HITBOX statement");
      return;
    }

    if (lexeme->value == "ON") {
      // ld a, 0    ; mode ON (all sprites)
      cpu.addLdA(0);
    } else if (lexeme->value == "OFF") {
      // ld a, 1    ; mode OFF (all sprites)
      cpu.addLdA(1);
    } else {
      // ld a, 2    ; mode AUTO (all sprites)
      cpu.addLdA(2);
    }

    // call set_sprite_hitbox
    context->getCodeOptimizer()->addKernelCall(DISP_set_sprite_hitbox);
    return;
  }

  if (t >= 2) {
    lexeme = action->actions[1]->lexeme;
    if (lexeme && lexeme->type == Lexeme::type_keyword &&
        (lexeme->value == "ON" || lexeme->value == "OFF" ||
         lexeme->value == "AUTO")) {
      if (t != 2) {
        context->syntaxError(
            "Wrong parameters count on SET SPRITE HITBOX statement");
        return;
      }
      if (lexeme->value == "ON") {
        mode = 3;
      } else if (lexeme->value == "OFF") {
        mode = 5;
      } else {
        mode = 6;
      }
    } else {
      mode = 4;
    }
  }

  if (mode == 4) {
    // push margins in reverse order (bottom, right, top, left) so that the
    // kernel can pop them as left, top, right, bottom; omitted trailing
    // margins default to zero
    for (i = 4; i >= 1; i--) {
      if (i < t) {
        result_subtype = expression.evalExpression(action->actions[i]);
        expression.addCast(result_subtype, Lexeme::subtype_numeric);
      } else {
        // ld hl, 0
        cpu.addLdHL(0x0000);
      }
      // push hl
      cpu.addPushHL();
    }
  }

  // sprite number
  result_subtype = expression.evalExpression(action->actions[0]);
  expression.addCast(result_subtype, Lexeme::subtype_numeric);

  // ld a, mode    ; sprite number is already in L
  cpu.addLdA(mode);

  // call set_sprite_hitbox
  context->getCodeOptimizer()->addKernelCall(DISP_set_sprite_hitbox);
}

void CompilerSetStatementStrategy::cmd_set_date(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->getCpu();
  auto& expression = *context->getExpressionEvaluator();
  shared_ptr<Lexeme> lexeme;
  shared_ptr<ActionNode> action = context->getCurrentAction()->actions[0];
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

    context->getCodeOptimizer()->addKernelCall(DISP_set_date);

  } else {
    context->syntaxError(
        "Wrong SET DATE parameters count.\nTry: SET DATE iYear, iMonth, "
        "iDay");
  }
}

void CompilerSetStatementStrategy::cmd_set_time(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->getCpu();
  auto& expression = *context->getExpressionEvaluator();
  shared_ptr<Lexeme> lexeme;
  shared_ptr<ActionNode> action = context->getCurrentAction()->actions[0];
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

    context->getCodeOptimizer()->addKernelCall(DISP_set_time);

  } else {
    context->syntaxError(
        "Wrong SET TIME parameters count.\nTry: SET TIME iHour, iMinute, "
        "iSecond");
  }
}

bool CompilerSetStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_set(context);
  return context->getCompiled();
}
