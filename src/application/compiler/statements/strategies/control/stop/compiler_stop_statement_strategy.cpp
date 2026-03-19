#include "compiler_stop_statement_strategy.h"

#include "action_node.h"
#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_end_statement_strategy.h"
#include "compiler_hooks.h"
#include "lexeme.h"

void CompilerStopStatementStrategy::cmd_stop(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  shared_ptr<ActionNode> action;
  shared_ptr<Lexeme> next_lexeme;
  unsigned int t = context->current_action->actions.size();

  if (t == 1) {
    action = context->current_action->actions[0];
    next_lexeme = action->lexeme;

    // ld hl, 0xFC6A    ; STOP state
    cpu.addLdHL(0xFC6A);

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
      context->syntaxError("Invalid STOP statement");
    }

  } else if (t == 0) {
    CompilerEndStatementStrategy endStmt;
    //! jump to the real END statement
    endStmt.execute(context);
  } else {
    context->syntaxError("Wrong number of parameters in STOP");
  }
}

bool CompilerStopStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  context->traps_checked = context->codeHelper->addCheckTraps();
  cmd_stop(context);
  return context->compiled;
}
