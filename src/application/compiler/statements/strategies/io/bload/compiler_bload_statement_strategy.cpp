#include "compiler_bload_statement_strategy.h"

#include "action_node.h"
#include "build_options.h"
#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include <strings.h>

#include "build_path_policy.h"
#include "lexeme.h"
#include "resources.h"

void CompilerBloadStatementStrategy::cmd_bload(
    shared_ptr<CompilerContext> context) {
  auto& cpu = *context->getCpu();
  auto& opts = *context->getOpts();
  shared_ptr<Lexeme> lexeme;
  shared_ptr<ActionNode> action;
  unsigned int t = context->getCurrentAction()->actions.size();
  bool isTinySprite;
  int resource_number;
  string filename, fileext;

  if (t == 2) {
    action = context->getCurrentAction()->actions[1];
    lexeme = action->lexeme;
    if (lexeme) {
      if (lexeme->name == "S") {
        action = context->getCurrentAction()->actions[0];
        lexeme = action->lexeme;
        if (lexeme) {
          if (lexeme->type == Lexeme::type_literal &&
              lexeme->subtype == Lexeme::subtype_string) {
            // add to resource list

            // verify file type (screen or sprite)

            filename = build_path::removeQuotes(lexeme->value);
            fileext = build_path::getFileExtension(filename);
            isTinySprite = (strcasecmp((char*)fileext.c_str(), ".SPR") == 0);

            resource_number = context->getResourceManager()->resources.size();
            lexeme->name = "FILE";
            context->getResourceManager()->addFile(filename, opts.inputPath);

            // execute a resource screen load

            // ld hl, resource number
            cpu.addLdHL(resource_number);

            // ld (DAC), hl
            cpu.addLdiiHL(def_DAC);

            if (isTinySprite) {
              // call cmd_wrtspr                    ; tiny sprite loader
              context->getCodeOptimizer()->addKernelCall(DISP_cmd_wrtspr);
            } else {
              // call screen_load
              context->getCodeOptimizer()->addKernelCall(DISP_cmd_screen_load);
            }

          } else {
            context->syntaxError(
                "BLOAD 1st parameter must be a string constant");
          }

        } else {
          context->syntaxError("BLOAD 1st parameter error");
        }

      } else {
        context->syntaxError("BLOAD valid only to screen");
      }

    } else {
      context->syntaxError("BLOAD 2nd parameter error");
    }

  } else {
    context->syntaxError("Invalid BLOAD parameters count");
  }
}

bool CompilerBloadStatementStrategy::execute(
    shared_ptr<CompilerContext> context) {
  cmd_bload(context);
  return context->getCompiled();
}
