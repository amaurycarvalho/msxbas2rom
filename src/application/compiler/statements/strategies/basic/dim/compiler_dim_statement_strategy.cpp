#include "compiler_dim_statement_strategy.h"

#include "compiler_context.h"
#include "lexeme.h"
#include "logger.h"

void CompilerDimStatementStrategy::cmd_dim(CompilerContext* context) {
  shared_ptr<Lexeme> lexeme, parm_lexeme;
  ActionNode* action;
  unsigned int i, k, w, tt, t = context->current_action->actions.size();
  int new_size;

  if (!t) {
    context->syntaxError("DIM parameters is missing");
  } else {
    for (i = 0; i < t; i++) {
      action = context->current_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->type == Lexeme::type_identifier) {
        if (lexeme->subtype == Lexeme::subtype_string)
          lexeme->x_factor = 256;
        else if (lexeme->subtype == Lexeme::subtype_numeric)
          lexeme->x_factor = 2;
        else if (lexeme->subtype == Lexeme::subtype_single_decimal ||
                 lexeme->subtype == Lexeme::subtype_double_decimal)
          lexeme->x_factor = 3;
        else
          lexeme->x_factor = 0;

        tt = action->actions.size();
        if (tt >= 1 && tt <= 2) {
          lexeme->isArray = true;

          if (tt == 1) {
            k = 0;
            w = 1;
          } else {
            k = 1;
            w = 0;
          }

          parm_lexeme = action->actions[k]->lexeme;
          if (parm_lexeme->type == Lexeme::type_literal &&
              parm_lexeme->subtype == Lexeme::subtype_numeric) {
            try {
              lexeme->x_size = stoi(parm_lexeme->value) + 1;
            } catch (exception& e) {
              context->logger->warning(
                  "Error while converting numeric constant " +
                  parm_lexeme->value);
              lexeme->x_size = 0;
            }
            if (!lexeme->x_size) {
              context->syntaxError("Array 1st dimension index cannot be zero");
              break;
            }
          } else {
            context->syntaxError(
                "Array 1st dimension index must be a integer constant");
            break;
          }

          if (tt == 2) {
            parm_lexeme = action->actions[w]->lexeme;
            if (parm_lexeme->type == Lexeme::type_literal &&
                parm_lexeme->subtype == Lexeme::subtype_numeric) {
              try {
                lexeme->y_size = stoi(parm_lexeme->value) + 1;
              } catch (exception& e) {
                context->logger->warning(
                    "Error while converting numeric constant " +
                    parm_lexeme->value);
                lexeme->y_size = 0;
              }
              if (!lexeme->y_size) {
                context->syntaxError(
                    "Array 2nd dimension index cannot be zero");
                break;
              }
            } else {
              context->syntaxError(
                  "Array 2nd dimension index must be a integer constant");
              break;
            }
          } else
            lexeme->y_size = 1;

          lexeme->y_factor = lexeme->x_factor * lexeme->x_size;
          new_size = lexeme->y_factor * lexeme->y_size;

          if (lexeme->array_size < new_size) lexeme->array_size = new_size;
        } else {
          context->syntaxError(
              "Arrays with more than 2 dimensions isn't supported");
          break;
        }

      } else {
        context->syntaxError("Invalid DIM parameter");
        break;
      }
    }
  }
}

bool CompilerDimStatementStrategy::execute(CompilerContext* context) {
  cmd_dim(context);
  return context->compiled;
}
