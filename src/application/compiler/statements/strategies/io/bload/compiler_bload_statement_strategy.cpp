#include "compiler_bload_statement_strategy.h"

#include "compiler_context.h"
#include "compiler_expression_evaluator.h"
#include "compiler_hooks.h"
#include "resources.h"

void CompilerBloadStatementStrategy::cmd_bload(CompilerContext* context) {
  auto& cpu = *context->cpu;
  auto& opts = *context->opts;
  Lexeme* lexeme;
  ActionNode* action;
  unsigned int t = context->current_action->actions.size();
  bool isTinySprite;
  int resource_number;
  string filename, fileext;

  if (t == 2) {
    action = context->current_action->actions[1];
    lexeme = action->lexeme;
    if (lexeme) {
      if (lexeme->name == "S") {
        action = context->current_action->actions[0];
        lexeme = action->lexeme;
        if (lexeme) {
          if (lexeme->type == Lexeme::type_literal &&
              lexeme->subtype == Lexeme::subtype_string) {
            // add to resource list

            // verify file type (screen or sprite)

            filename = removeQuotes(lexeme->value);
            fileext = getFileExtension(filename);
            isTinySprite = (strcasecmp((char*)fileext.c_str(), ".SPR") == 0);

            resource_number = context->resourceManager->resources.size();
            lexeme->name = "FILE";
            context->resourceManager->addFile(filename, opts.inputPath);

            // execute a resource screen load

            // ld hl, resource number
            cpu.addLdHL(resource_number);

            // ld (DAC), hl
            cpu.addLdiiHL(def_DAC);

            if (isTinySprite) {
              // call cmd_wrtspr                    ; tiny sprite loader
              cpu.addCall(def_cmd_wrtspr);
            } else {
              // call screen_load
              cpu.addCall(def_cmd_screen_load);
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

bool CompilerBloadStatementStrategy::execute(CompilerContext* context) {
  cmd_bload(context);
  return context->compiled;
}
