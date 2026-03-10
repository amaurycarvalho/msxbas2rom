#include "compiler_cmd_runbas_handler.h"

#include "action_node.h"
#include "compiler_context.h"

bool CompilerCmdRunBasHandler::execute(CompilerContext* context,
                                       ActionNode* action) {
  context->syntaxError("CMD RUNBAS not implemented yet");
  return context->compiled;
}
