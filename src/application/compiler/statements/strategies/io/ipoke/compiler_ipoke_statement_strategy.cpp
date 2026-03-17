#include "compiler_ipoke_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_expression_evaluator.h"

void CompilerIpokeStatementStrategy::cmd_ipoke(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<Lexeme> lexeme;
  ActionNode* action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t == 2) {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;
      if (lexeme) {
        result_subtype = expression.evalExpression(action);

        // cast
        expression.addCast(result_subtype, Lexeme::subtype_numeric);

        if (i == 0) {
          // push hl
          cpu.addPushHL();
        } else {
          // ex de, hl
          cpu.addExDEHL();
          // pop hl
          cpu.addPopHL();
          // ld (hl), e
          cpu.addLdiHLE();
          // inc hl
          cpu.addIncHL();
          // ld (hl), d
          cpu.addLdiHLD();
        }
      }
    }

  } else {
    context->syntaxError("Invalid IPOKE parameters");
  }
}

bool CompilerIpokeStatementStrategy::execute(CompilerContext* context) {
  cmd_ipoke(context);
  return context->compiled;
}
