#include "compiler_strig_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

void CompilerStrigStatementStrategy::cmd_strig(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<ActionNode> action1, action2;
  shared_ptr<Lexeme> next_lexeme;
  unsigned int t = context->current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    action2 = context->current_action->actions[1];
    next_lexeme = action2->lexeme;

    // ld hl, strig number
    action1 = context->current_action->actions[0];
    result_subtype = expression.evalExpression(action1);
    expression.addCast(result_subtype, Lexeme::subtype_numeric);

    // dec hl
    // cpuOpcodeWriter->addDecHL();

    // ld e, l
    cpu.addLdEL();
    // ld d, h
    cpu.addLdDH();
    // add hl, de
    cpu.addAddHLDE();
    // add hl, de       ; strig number * 3
    cpu.addAddHLDE();

    // ld de, 0xFC70    ; STRIG state position = key number * 3 + 0xFC70
    cpu.addLdDE(0xFC70);

    // add hl, de
    cpu.addAddHLDE();

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
      context->syntaxError("Invalid STRIG statement");
    }

  } else {
    context->syntaxError("Wrong number of parameters in STRIG statement");
  }
}

bool CompilerStrigStatementStrategy::execute(CompilerContext* context) {
  cmd_strig(context);
  return context->compiled;
}
