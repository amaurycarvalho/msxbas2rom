#include "compiler_cmd_rsctoram_handler.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "lexeme.h"

bool CompilerCmdRscToRamHandler::execute(shared_ptr<CompilerContext> context,
                                         shared_ptr<ActionNode> action) {
  auto& cpu = *context->getCpu();
  auto& expression = *context->getExpressionEvaluator();

  const int n = static_cast<int>(action->actions.size());
  if (n < 2 || n > 5) {
    context->syntaxError("CMD RSCTORAM syntax error");
    return context->getCompiled();
  }

  auto evaluateWord = [&](int index, int address) {
    if (index < n) {
      auto sub = action->actions[index];
      int subtype = expression.evalExpression(sub);
      expression.addCast(subtype, Lexeme::subtype_numeric);
    } else {
      cpu.addLdHL(0);
    }
    cpu.addLdiiHL(address);
  };

  evaluateWord(0, def_RSC_RESOURCE);
  evaluateWord(1, def_RSC_DEST);
  evaluateWord(2, def_RSC_PLETTER);
  evaluateWord(3, def_RSC_OFFSET);
  evaluateWord(4, def_RSC_SIZE);

  context->getCodeOptimizer()->addKernelCall(DISP_cmd_rsctoram);

  return context->getCompiled();
}
