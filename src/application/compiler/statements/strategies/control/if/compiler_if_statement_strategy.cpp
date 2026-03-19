#include "compiler_if_statement_strategy.h"

#include "action_node.h"
#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_evaluator.h"
#include "compiler_expression_evaluator.h"
#include "compiler_fixup_resolver.h"
#include "compiler_symbol_resolver.h"
#include "fix_node.h"
#include "lexeme.h"

void CompilerIfStatementStrategy::cmd_if(shared_ptr<CompilerContext> context) {
  auto& cpu = *context->cpu;
  auto& fixup = *context->fixupResolver;
  auto& expression = *context->expressionEvaluator;
  auto& evaluator = *context->evaluator;
  shared_ptr<Lexeme> lexeme, last_lexeme;
  shared_ptr<ActionNode> action, last_action;
  shared_ptr<ActionNode> saved_action = context->current_action;
  unsigned int i, t = saved_action->actions.size(), tt;
  int result_subtype;
  shared_ptr<FixNode> mark_else = nullptr, mark_endif = nullptr;
  bool isLastActionGoto = false, isElseLikeEndif = true;

  if (!t) {
    context->syntaxError("IF parameters is missing");
  } else {
    for (i = 0; i < t; i++) {
      action = saved_action->actions[i];
      lexeme = action->lexeme;

      if (lexeme->type == Lexeme::type_keyword) {
        if (lexeme->value == "COND") {
          // ld hl, data parameter

          result_subtype = expression.evalExpression(action->actions[0]);

          if (result_subtype == Lexeme::subtype_numeric) {
            // ld a, l
            cpu.addLdAL();
            // or h
            cpu.addOrH();
            // jp z, ELSE or ENDIF
            mark_else = fixup.addMark();
            cpu.addJpZ(0x0000);

          } else {
            context->syntaxError("Invalid condition expression");
            break;
          }

        } else if (lexeme->value == "THEN") {
          tt = action->actions.size();
          if (tt) {
            last_action = action->actions[tt - 1];
            last_lexeme = last_action->lexeme;
            isLastActionGoto = (last_lexeme->type == Lexeme::type_keyword &&
                                last_lexeme->value == "GOTO");
          }

          if (!evaluator.evalActions(action)) break;

        } else if (lexeme->value == "GOTO") {
          isLastActionGoto = true;

          if (!evaluator.evalAction(action)) break;

        } else if (lexeme->value == "GOSUB") {
          if (!evaluator.evalAction(action)) break;

        } else if (lexeme->value == "ELSE") {
          if (!isLastActionGoto) {
            // jp ENDIF
            mark_endif = fixup.addMark();
            cpu.addJp(0x0000);
          }

          isLastActionGoto = false;
          isElseLikeEndif = false;

          tt = action->actions.size();
          if (tt == 1) {
            last_action = action->actions[0];
            last_lexeme = last_action->lexeme;
            isLastActionGoto = (last_lexeme->type == Lexeme::type_keyword &&
                                last_lexeme->value == "GOTO");
          }

          if (isLastActionGoto) {
            if (last_action->actions.size()) {
              last_lexeme = last_action->actions[0]->lexeme;
            }

            if (last_lexeme->type == Lexeme::type_literal &&
                last_lexeme->subtype == Lexeme::subtype_numeric) {
              // mark ELSE position
              if (mark_else) {
                mark_else->symbol =
                    context->symbolResolver->addSymbol(last_lexeme->value);
              } else {
                context->syntaxError("ELSE parameter is missing");
              }
            } else {
              context->syntaxError("Invalid GOTO parameter");
            }

          } else {
            // mark ELSE position
            if (mark_else)
              mark_else->symbol->address = cpu.context->code_pointer;

            if (!evaluator.evalActions(action)) break;
          }

        } else {
          context->syntaxError("Invalid IF syntax");
          break;
        }

      } else {
        context->syntaxError("Invalid IF parameter type");
        break;
      }
    }

    // mark ENDIF position
    if (mark_endif)
      mark_endif->symbol->address = cpu.context->code_pointer;
    else if (mark_else)
      if (isElseLikeEndif)
        mark_else->symbol->address = cpu.context->code_pointer;
  }
}

bool CompilerIfStatementStrategy::execute(shared_ptr<CompilerContext> context) {
  context->traps_checked = context->codeHelper->addCheckTraps();
  cmd_if(context);
  return context->compiled;
}
