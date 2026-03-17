#include "compiler_sprite_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

void CompilerSpriteStatementStrategy::cmd_sprite(CompilerContext* context) {
  auto& cpu = *context->cpu;
  shared_ptr<ActionNode> action;
  shared_ptr<Lexeme> next_lexeme;
  unsigned int t = context->current_action->actions.size();

  if (t == 1) {
    action = context->current_action->actions[0];
    next_lexeme = action->lexeme;

    if (next_lexeme->value == "LOAD") {
      context->current_action = action;
      cmd_sprite_load(context);
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

void CompilerSpriteStatementStrategy::cmd_sprite_load(
    CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<ActionNode> action;
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

bool CompilerSpriteStatementStrategy::execute(CompilerContext* context) {
  cmd_sprite(context);
  return context->compiled;
}
