#include "compiler_draw_statement_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "fix_node.h"
#include "lexeme.h"

void CompilerDrawStatementStrategy::cmd_draw(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->getCpu();
  auto& fixup = *context->getFixupResolver();
  auto& expression = *context->getExpressionEvaluator();
  shared_ptr<Lexeme> lexeme;
  shared_ptr<ActionNode> action;
  unsigned int i, t = context->getCurrentAction()->actions.size();
  int result_subtype;

  if (t == 1) {
    i = 0;
    action = context->getCurrentAction()->actions[i];
    lexeme = action->lexeme;
    if (lexeme) {
      result_subtype = expression.evalExpression(action);

      if (result_subtype == Lexeme::subtype_string) {
        // call draw function
        if (context->getDrawStmtMark())
          fixup.addFix(context->getDrawStmtMark()->symbol);
        else
          context->getDrawStmtMark() = fixup.addMark();
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

bool CompilerDrawStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_draw(context);
  return context->getCompiled();
}
