#include "compiler_defusr_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"

void CompilerDefUsrStatementStrategy::cmd_defusr(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<ActionNode> action, subaction;
  unsigned int t;
  int result_subtype;

  action = context->current_action->actions[0];
  t = action->actions.size();

  if (t == 2) {
    context->has_defusr = true;

    subaction = action->actions[0];
    result_subtype = expression.evalExpression(subaction);
    expression.addCast(result_subtype, Lexeme::subtype_numeric);

    // push hl
    cpu.addPushHL();

    subaction = action->actions[1];
    result_subtype = expression.evalExpression(subaction);
    expression.addCast(result_subtype, Lexeme::subtype_numeric);

    // ex de,hl
    cpu.addExDEHL();

    // pop hl
    cpu.addPopHL();
    // ld bc, USRTAB
    cpu.addLdBC(def_USRTAB);
    // add hl,hl
    cpu.addAddHLHL();
    // add hl,bc
    cpu.addAddHLBC();
    // ld (hl), e
    cpu.addLdiHLE();
    // inc hl
    cpu.addIncHL();
    // ld (hl), d
    cpu.addLdiHLD();

  } else {
    context->syntaxError("Wrong DEF USR parameters count");
  }
}

bool CompilerDefUsrStatementStrategy::execute(CompilerContext* context) {
  cmd_defusr(context);
  return context->compiled;
}
