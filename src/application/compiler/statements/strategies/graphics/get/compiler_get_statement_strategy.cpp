#include "compiler_get_statement_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "compiler_variable_emitter.h"
#include "lexeme.h"

void CompilerGetStatementStrategy::cmd_get(
    shared_ptr<CompilerContext> context) {
  shared_ptr<ActionNode> action;
  shared_ptr<Lexeme> next_lexeme;
  unsigned int t = context->current_action->actions.size();

  if (t == 1) {
    action = context->current_action->actions[0];
    next_lexeme = action->lexeme;

    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "DATE") {
        cmd_get_date(context);
      } else if (next_lexeme->value == "TIME") {
        cmd_get_time(context);
      } else if (next_lexeme->value == "TILE") {
        cmd_get_tile(context);
      } else if (next_lexeme->value == "SPRITE") {
        cmd_get_sprite(context);
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

void CompilerGetStatementStrategy::cmd_get_date(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  shared_ptr<Lexeme> lexeme;
  shared_ptr<ActionNode> action = context->current_action->actions[0];
  shared_ptr<ActionNode> sub_action;
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

void CompilerGetStatementStrategy::cmd_get_time(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  shared_ptr<Lexeme> lexeme;
  shared_ptr<ActionNode> action = context->current_action->actions[0];
  shared_ptr<ActionNode> sub_action;
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

void CompilerGetStatementStrategy::cmd_get_tile(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
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

void CompilerGetStatementStrategy::cmd_get_sprite(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
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

bool CompilerGetStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_get(context);
  return context->compiled;
}
