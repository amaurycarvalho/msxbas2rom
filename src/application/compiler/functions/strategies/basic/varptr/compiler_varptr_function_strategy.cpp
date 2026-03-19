#include "compiler_varptr_function_strategy.h"

#include "action_node.h"
#include "compiler_context.h"
#include "compiler_variable_emitter.h"
#include "lexeme.h"

int VarptrCompilerFunctionStrategy::execute(shared_ptr<CompilerContext> context,
                                            shared_ptr<ActionNode> action,
                                            int* result,
                                            unsigned int parmCount) {
  if (!context || !action || !action->lexeme) return Lexeme::subtype_unknown;
  if (parmCount < 1) return Lexeme::subtype_unknown;
  if (action->lexeme->value != "VARPTR") return Lexeme::subtype_unknown;

  (void)result;

  shared_ptr<ActionNode> next_action = action->actions[0];
  if (next_action && next_action->lexeme &&
      next_action->lexeme->type == Lexeme::type_identifier) {
    context->variableEmitter->addVarAddress(next_action);
    return Lexeme::subtype_numeric;
  }

  return Lexeme::subtype_unknown;
}
