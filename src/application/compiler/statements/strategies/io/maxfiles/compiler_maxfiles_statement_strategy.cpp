#include "compiler_maxfiles_statement_strategy.h"

#include "action_node.h"
#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_hooks.h"
#include "fix_node.h"
#include "lexeme.h"

void CompilerMaxfilesStatementStrategy::cmd_maxfiles(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<ActionNode> action;
  unsigned int t = context->current_action->actions.size();
  int result_subtype;

  if (t) {
    action = context->current_action->actions[0];
    result_subtype = expression.evalExpression(action);
    expression.addCast(result_subtype, Lexeme::subtype_numeric);

    // ld a, (MAXFIL)
    cpu.addLdAii(def_MAXFIL);
    // cp l
    cpu.addCpL();
    // ld a, l
    cpu.addLdAL();
    // call nz, cmd_maxfiles
    cpu.addCallNZ(def_cmd_maxfiles);

  } else {
    context->syntaxError("Empty MAXFILES assignment");
  }
}

bool CompilerMaxfilesStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_maxfiles(context);
  return context->compiled;
}
