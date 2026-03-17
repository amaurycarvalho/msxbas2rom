#include "compiler_draw_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"

void CompilerDrawStatementStrategy::cmd_draw(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
  shared_ptr<Lexeme> lexeme;
  ActionNode* action;
  unsigned int i, t = context->current_action->actions.size();
  int result_subtype;

  if (t == 1) {
    i = 0;
    action = context->current_action->actions[i];
    lexeme = action->lexeme;
    if (lexeme) {
      result_subtype = expression.evalExpression(action);

      if (result_subtype == Lexeme::subtype_string) {
        // call draw function
        if (context->draw_mark)
          fixup.addFix(context->draw_mark->symbol);
        else
          context->draw_mark = fixup.addMark();
        cpu.addCall(0x0000);

      } else {
        context->syntaxError("Invalid DRAW parameter");
        return;
      }
    }

  } else {
    context->syntaxError("Invalid DRAW parameters");
  }
}

bool CompilerDrawStatementStrategy::execute(CompilerContext* context) {
  cmd_draw(context);
  return context->compiled;
}
