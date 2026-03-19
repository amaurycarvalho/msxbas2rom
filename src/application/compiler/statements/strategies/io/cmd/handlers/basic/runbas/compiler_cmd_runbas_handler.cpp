#include "compiler_cmd_runbas_handler.h"

#include "action_node.h"
#include "compiler_context.h"

bool CompilerCmdRunBasHandler::execute(shared_ptr<CompilerContext> context,
                                       shared_ptr<ActionNode> action) {
  context->syntaxError("CMD RUNBAS not implemented yet");
  return context->compiled;
}
