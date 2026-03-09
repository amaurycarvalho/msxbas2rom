#include "compiler_iread_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "compiler_variable_emitter.h"

void CompilerIreadStatementStrategy::cmd_iread(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  auto& variable = *context->variableEmitter;
  ActionNode* action;
  Lexeme* lexeme;
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

bool CompilerIreadStatementStrategy::execute(CompilerContext* context) {
  cmd_iread(context);
  return context->compiled;
}
