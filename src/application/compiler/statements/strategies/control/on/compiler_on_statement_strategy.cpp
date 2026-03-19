#include "compiler_on_statement_strategy.h"

#include "action_node.h"
#include "build_options.h"
#include "compiler_code_helper.h"
#include "compiler_code_optimizer.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "fix_node.h"
#include "lexeme.h"

void CompilerOnStatementStrategy::cmd_on(shared_ptr<CompilerContext> context) {
  shared_ptr<ActionNode> action;
  shared_ptr<Lexeme> next_lexeme;
  unsigned int t = context->current_action->actions.size();

  if (t) {
    action = context->current_action->actions[0];
    next_lexeme = action->lexeme;

    if (next_lexeme->type == Lexeme::type_keyword &&
        next_lexeme->value == "ERROR") {
      cmd_on_error(context);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "INTERVAL") {
      cmd_on_interval(context);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "KEY") {
      cmd_on_key(context);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "SPRITE") {
      cmd_on_sprite(context);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "STOP") {
      cmd_on_stop(context);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "STRIG") {
      cmd_on_strig(context);
    } else if (next_lexeme->type == Lexeme::type_keyword &&
               next_lexeme->value == "INDEX") {
      cmd_on_goto_gosub(context);
    } else {
      context->syntaxError("Invalid ON statement");
    }

  } else {
    context->syntaxError("Empty ON statement");
  }
}

void CompilerOnStatementStrategy::cmd_on_error(
    shared_ptr<CompilerContext> context) {
  context->syntaxError("Not implemented yet");
}

void CompilerOnStatementStrategy::cmd_on_interval(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& opts = *context->opts;
  auto& expression = *context->expressionEvaluator;
  auto& optimizer = *context->codeOptimizer;
  shared_ptr<ActionNode> action, sub_action, parm_action;
  shared_ptr<Lexeme> lexeme, parm_lexeme;
  unsigned int t;
  int result_subtype;

  action = context->current_action->actions[0];
  t = action->actions.size();

  if (t == 2) {
    // INDEX VARIABLE

    sub_action = action->actions[0];
    lexeme = sub_action->lexeme;
    if (lexeme->value != "INDEX") {
      context->syntaxError("Interval index is missing in ON INTERVAL");
      return;
    }
    if (sub_action->actions.size() != 1) {
      context->syntaxError(
          "Wrong parameter count in interval index from ON INTERVAL");
      return;
    }

    parm_action = sub_action->actions[0];

    // ld hl, variable
    result_subtype = expression.evalExpression(parm_action);

    expression.addCast(result_subtype, Lexeme::subtype_numeric);

    // di
    cpu.addDI();
    //   ld (0xFCA0), hl   ; INTVAL
    cpu.addLdiiHL(0xFCA0);
    //   xor a
    cpu.addXorA();
    //   ld (0xFC7F), a    ; ON INTERVAL STATE (0=off, 1=on)
    cpu.addLdiiA(0xFC7F);
    //   ld (0xFCA3), a    ; INTCNT - initialize with zero (2 bytes)
    cpu.addLdiiA(0xFCA3);
    cpu.addLdiiA(0xFCA4);
    // ei
    cpu.addEI();

    // GOSUB

    sub_action = action->actions[1];
    lexeme = sub_action->lexeme;
    if (lexeme->value != "GOSUB") {
      context->syntaxError("GOSUB is missing in ON INTERVAL");
      return;
    }
    if (sub_action->actions.size() != 1) {
      context->syntaxError("Wrong parameter count in GOSUB from ON INTERVAL");
      return;
    }

    parm_action = sub_action->actions[0];
    parm_lexeme = parm_action->lexeme;

    if (parm_lexeme->type == Lexeme::type_literal &&
        parm_lexeme->subtype == Lexeme::subtype_numeric) {
      if (opts.megaROM) {
        // ld hl, GOSUB ADDRESS
        fixup.addFix(parm_lexeme->value);
        optimizer.addLdHLmegarom();
        // ld (0xFC80), hl                ; INTERVAL ADDRESS
        cpu.addLdiiHL(0xFC80);
        // ld (MR_TRAP_SEGMS+17), a       ; INTERVAL segment
        cpu.addLdiiA(def_MR_TRAP_SEGMS + 17);

      } else {
        // ld hl, GOSUB ADDRESS
        fixup.addFix(parm_lexeme->value);
        cpu.addLdHL(0x0000);
        // ld (0xFC80), hl   ; GOSUB ADDRESS
        cpu.addLdiiHL(0xFC80);
      }

    } else {
      context->syntaxError("Invalid GOSUB parameter in ON INTERVAL");
      return;
    }

  } else {
    context->syntaxError("ON INTERVAL with empty parameters");
  }
}

void CompilerOnStatementStrategy::cmd_on_key(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& opts = *context->opts;
  auto& optimizer = *context->codeOptimizer;
  shared_ptr<ActionNode> action, sub_action;
  shared_ptr<Lexeme> lexeme, sub_lexeme;
  unsigned int i, t;

  action = context->current_action->actions[0];
  if (action->actions.size() != 1) {
    context->syntaxError("Wrong parameters in ON KEY");
    return;
  }

  action = action->actions[0];
  lexeme = action->lexeme;
  if (lexeme->value != "GOSUB") {
    context->syntaxError("GOSUB parameters is missing in ON KEY");
    return;
  }
  t = action->actions.size();

  if (t) {
    // GOSUB LIST

    // ld hl, 0xFC4D    ; KEY first GOSUB position = 0xFC4C+1
    cpu.addLdHL(0xfc4D);

    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      sub_lexeme = sub_action->lexeme;

      if (sub_lexeme->type == Lexeme::type_literal &&
          sub_lexeme->subtype == Lexeme::subtype_numeric) {
        if (opts.megaROM) {
          // push hl
          cpu.addPushHL();
          //   ld hl, GOSUB ADDRESS
          fixup.addFix(sub_lexeme->value);
          optimizer.addLdHLmegarom();
          //   ld (MR_TRAP_SEGMS), a       ; KEY segment
          cpu.addLdiiA(def_MR_TRAP_SEGMS + i);
          //   ex de, hl
          cpu.addExDEHL();
          // pop hl
          cpu.addPopHL();

        } else {
          // ld de, call address
          fixup.addFix(sub_lexeme->value);
          cpu.addLdDE(0x0000);
        }

      } else {
        // ld hl, 0x368D   ; dummy bios RET address
        cpu.addLdHL(def_XBASIC_DUMMY_RET);
      }

      // ld (hl), e
      cpu.addLdiHLE();
      // inc hl
      cpu.addIncHL();
      // ld (hl), d
      cpu.addLdiHLD();
      // inc hl
      cpu.addIncHL();
      // inc hl
      cpu.addIncHL();
    }

  } else {
    context->syntaxError("ON KEY with empty parameters");
  }
}

void CompilerOnStatementStrategy::cmd_on_sprite(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& opts = *context->opts;
  auto& optimizer = *context->codeOptimizer;
  shared_ptr<ActionNode> action, sub_action;
  shared_ptr<Lexeme> lexeme, sub_lexeme;
  unsigned int t, i = 0;

  action = context->current_action->actions[0];
  if (action->actions.size() != 1) {
    context->syntaxError("Wrong parameters in ON SPRITE");
    return;
  }

  action = action->actions[0];
  lexeme = action->lexeme;
  if (lexeme->value != "GOSUB") {
    context->syntaxError("GOSUB parameters is missing in ON SPRITE");
    return;
  }
  t = action->actions.size();

  if (t == 1) {
    // GOSUB address

    sub_action = action->actions[0];
    sub_lexeme = sub_action->lexeme;

    if (sub_lexeme->type == Lexeme::type_literal &&
        sub_lexeme->subtype == Lexeme::subtype_numeric) {
      if (opts.megaROM) {
        // push hl
        cpu.addPushHL();
        //   ld hl, GOSUB ADDRESS
        fixup.addFix(sub_lexeme->value);
        optimizer.addLdHLmegarom();
        //   ld (MR_TRAP_SEGMS+11), a       ; SPRITE segment
        cpu.addLdiiA(def_MR_TRAP_SEGMS + 11 + i);
        //   ex de, hl
        cpu.addExDEHL();
        // pop hl
        cpu.addPopHL();

      } else {
        // ld hl, call address
        fixup.addFix(sub_lexeme->value);
        cpu.addLdHL(0x0000);
      }

    } else {
      // ld hl, 0x368D   ; dummy bios RET address
      cpu.addLdHL(def_XBASIC_DUMMY_RET);
    }

    // ld (0xFC6E), hl     ; STOP GOSUB position = 0xFC6D+1
    cpu.addLdiiHL(0xFC6E);

  } else {
    context->syntaxError("ON SPRITE with wrong count of parameters");
  }
}

void CompilerOnStatementStrategy::cmd_on_stop(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& opts = *context->opts;
  auto& optimizer = *context->codeOptimizer;
  shared_ptr<ActionNode> action, sub_action;
  shared_ptr<Lexeme> lexeme, sub_lexeme;
  unsigned int t, i = 0;

  action = context->current_action->actions[0];
  if (action->actions.size() != 1) {
    context->syntaxError("Wrong parameters in ON STOP");
    return;
  }

  action = action->actions[0];
  lexeme = action->lexeme;
  if (lexeme->value != "GOSUB") {
    context->syntaxError("GOSUB parameters is missing in ON STOP");
    return;
  }
  t = action->actions.size();

  if (t == 1) {
    // GOSUB address

    sub_action = action->actions[0];
    sub_lexeme = sub_action->lexeme;

    if (sub_lexeme->type == Lexeme::type_literal &&
        sub_lexeme->subtype == Lexeme::subtype_numeric) {
      if (opts.megaROM) {
        // push hl
        cpu.addPushHL();
        //   ld hl, GOSUB ADDRESS
        fixup.addFix(sub_lexeme->value);
        optimizer.addLdHLmegarom();
        //   ld (MR_TRAP_SEGMS+10), a       ; STOP segment
        cpu.addLdiiA(def_MR_TRAP_SEGMS + 10 + i);
        //   ex de, hl
        cpu.addExDEHL();
        // pop hl
        cpu.addPopHL();

      } else {
        // ld hl, call address
        fixup.addFix(sub_lexeme->value);
        cpu.addLdHL(0x0000);
      }

    } else {
      // ld hl, 0x368D   ; dummy bios RET address
      cpu.addLdHL(def_XBASIC_DUMMY_RET);
    }

    // ld (0xFC6B), hl     ; STOP GOSUB position = 0xFC6A+1
    cpu.addLdiiHL(0xfc6B);

  } else {
    context->syntaxError("ON STOP with wrong count of parameters");
  }
}

void CompilerOnStatementStrategy::cmd_on_strig(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& opts = *context->opts;
  auto& optimizer = *context->codeOptimizer;
  shared_ptr<ActionNode> action, sub_action;
  shared_ptr<Lexeme> lexeme, sub_lexeme;
  unsigned int i, t;

  action = context->current_action->actions[0];
  if (action->actions.size() != 1) {
    context->syntaxError("Wrong parameters in ON STRIG");
    return;
  }

  action = action->actions[0];
  lexeme = action->lexeme;
  if (lexeme->value != "GOSUB") {
    context->syntaxError("GOSUB parameters is missing in ON STRIG");
    return;
  }
  t = action->actions.size();

  if (t) {
    // GOSUB LIST

    // ld hl, 0xFC71    ; STRIG first GOSUB position = 0xFC70+1
    cpu.addLdHL(0xFC71);

    for (i = 0; i < t; i++) {
      sub_action = action->actions[i];
      sub_lexeme = sub_action->lexeme;

      if (sub_lexeme->type == Lexeme::type_literal &&
          sub_lexeme->subtype == Lexeme::subtype_numeric) {
        if (opts.megaROM) {
          // push hl
          cpu.addPushHL();
          //   ld hl, GOSUB ADDRESS
          fixup.addFix(sub_lexeme->value);
          optimizer.addLdHLmegarom();
          //   ld (MR_TRAP_SEGMS+12), a       ; STRIG segment
          cpu.addLdiiA(def_MR_TRAP_SEGMS + 12 + i);
          //   ex de, hl
          cpu.addExDEHL();
          // pop hl
          cpu.addPopHL();

        } else {
          // ld de, call address
          fixup.addFix(sub_lexeme->value);
          cpu.addLdDE(0x0000);
        }

      } else {
        // ld de, 0x368D   ; dummy bios RET address
        cpu.addLdDE(def_XBASIC_DUMMY_RET);
      }

      // ld (hl), e
      cpu.addLdiHLE();
      // inc hl
      cpu.addIncHL();
      // ld (hl), d
      cpu.addLdiHLD();
      // inc hl
      cpu.addIncHL();
      // inc hl
      cpu.addIncHL();
    }

  } else {
    context->syntaxError("ON STRIG with empty parameters");
  }
}

void CompilerOnStatementStrategy::cmd_on_goto_gosub(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& opts = *context->opts;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<ActionNode> action, sub_action;
  shared_ptr<Lexeme> lexeme, sub_lexeme;
  shared_ptr<FixNode> mark;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    // INDEX VARIABLE
    // ld hl, variable

    action = context->current_action->actions[0];
    if (action->actions.size() == 0) {
      context->syntaxError("Empty parameter in ON GOTO/GOSUB");
      return;
    }

    sub_action = action->actions[0];

    result_subtype = expression.evalExpression(sub_action);

    expression.addCast(result_subtype, Lexeme::subtype_numeric);

    // GOTO / GOSUB LIST

    action = context->current_action->actions[1];
    lexeme = action->lexeme;
    t = action->actions.size();

    // ld a, l
    cpu.addLdAL();

    if (opts.megaROM) {
      // ld (TEMP), a
      cpu.addLdiiA(def_TEMP);
    }

    // and a
    cpu.addAndA();
    // jp z, address
    mark = fixup.addMark();
    cpu.addJpZ(0x0000);

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
          if (opts.megaROM) {
            // ld a, (TEMP)
            cpu.addLdAii(def_TEMP);
          }

          // dec a
          cpu.addDecA();

          if (opts.megaROM) {
            // ld (TEMP), a
            cpu.addLdiiA(def_TEMP);
          }

          // jp z, address
          fixup.addFix(sub_lexeme->value);
          cpu.addJpZ(0x0000);

        } else {
          if (opts.megaROM) {
            // ld a, (TEMP)
            cpu.addLdAii(def_TEMP);
          }

          // dec a
          cpu.addDecA();

          if (opts.megaROM) {
            // ld (TEMP), a
            cpu.addLdiiA(def_TEMP);
          }

          if (opts.megaROM) {
            // jr nz, $+25
            cpu.addJrNZ(24);
          } else {
            // jr nz, $+7
            cpu.addJrNZ(0x06);
          }
          //   call address
          fixup.addFix(sub_lexeme->value);
          cpu.addCall(0x0000);
          //   jp address
          fixup.addFix(mark->symbol);
          cpu.addJp(0x0000);
        }

      } else {
        if (opts.megaROM) {
          // ld a, (TEMP)
          cpu.addLdAii(def_TEMP);
        }

        // dec a
        cpu.addDecA();

        if (opts.megaROM) {
          // ld (TEMP), a
          cpu.addLdiiA(def_TEMP);
        }
      }
    }

    mark->symbol->address = cpu.context->code_pointer;

  } else {
    context->syntaxError("ON GOTO/GOSUB with empty parameters");
  }
}

bool CompilerOnStatementStrategy::execute(shared_ptr<CompilerContext> context) {
  context->traps_checked = context->codeHelper->addCheckTraps();
  cmd_on(context);
  return context->compiled;
}
