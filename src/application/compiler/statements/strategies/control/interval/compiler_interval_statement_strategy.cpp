#include "compiler_interval_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_hooks.h"

void CompilerIntervalStatementStrategy::cmd_interval(CompilerContext* context) {
  auto& cpu = *context->cpu;
  shared_ptr<ActionNode> action;
  shared_ptr<Lexeme> next_lexeme;
  unsigned int t = context->current_action->actions.size();

  if (t == 1) {
    action = context->current_action->actions[0];
    next_lexeme = action->lexeme;

    // ld hl, 0xFC7F    ; INTERVAL state
    cpu.addLdHL(0xfc7f);

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
      context->syntaxError("Invalid INTERVAL statement");
    }

  } else {
    context->syntaxError("Empty INTERVAL statement");
  }
}

bool CompilerIntervalStatementStrategy::execute(CompilerContext* context) {
  cmd_interval(context);
  return context->compiled;
}
