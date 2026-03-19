#include "compiler_read_statement_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "compiler_variable_emitter.h"
#include "lexeme.h"

void CompilerReadStatementStrategy::cmd_read(
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
        context->syntaxError("Invalid READ parameter type");
        return;
      }

      // call read
      cpu.addCall(def_XBASIC_READ);

      expression.addCast(Lexeme::subtype_string, lexeme->subtype);

      // do assignment

      variable.addAssignment(action);
    }

  } else {
    context->syntaxError("READ with empty parameters");
  }
}

bool CompilerReadStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_read(context);
  return context->compiled;
}
