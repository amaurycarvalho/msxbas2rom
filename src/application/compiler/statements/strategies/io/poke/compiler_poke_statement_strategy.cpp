#include "compiler_poke_statement_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "lexeme.h"

void CompilerPokeStatementStrategy::cmd_poke(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<Lexeme> lexeme;
  shared_ptr<ActionNode> action;
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
        }
      }
    }

  } else {
    context->syntaxError("Invalid POKE parameters");
  }
}

bool CompilerPokeStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_poke(context);
  return context->compiled;
}
