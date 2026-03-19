#include "compiler_iread_statement_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "compiler_variable_emitter.h"
#include "lexeme.h"

void CompilerIreadStatementStrategy::cmd_iread(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& variable = *context->variableEmitter;
  shared_ptr<ActionNode> action;
  shared_ptr<Lexeme> lexeme;
  unsigned int i, t = context->current_action->actions.size();

  if (t) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->type != Lexeme::type_identifier) {
        context->syntaxError("Invalid IREAD parameter type");
        return;
      }

      // call XBASIC_IREAD
      cpu.addCall(def_XBASIC_IREAD);

      expression.addCast(Lexeme::subtype_numeric, lexeme->subtype);

      // do assignment

      variable.addAssignment(action);
    }

  } else {
    context->syntaxError("IREAD with empty parameters");
  }
}

bool CompilerIreadStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_iread(context);
  return context->compiled;
}
