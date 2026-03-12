/***
 * @file compiler_evaluator.cpp
 * @brief Compiler evaluator
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_evaluator.h"

#include "action_node.h"
#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "compiler_statement_strategy.h"
#include "compiler_statement_strategy_factory.h"
#include "tag_node.h"

CompilerEvaluator::CompilerEvaluator(CompilerContext* context) {
  this->context = context;
  statementFactory.reset(new CompilerStatementStrategyFactory());
}

CompilerEvaluator::~CompilerEvaluator() = default;

bool CompilerEvaluator::evaluate(TagNode* tag) {
  ActionNode* action;
  unsigned int i, t = tag->actions.size(), lin;

  context->current_tag = tag;

  if (context->opts->lineNumber) {
    try {
      lin = stoi(tag->name);
    } catch (exception& e) {
      printf("Warning: error while converting numeric constant %s\n",
             tag->name.c_str());
      lin = 0;
    }
    // ld hl, line number
    context->cpu->addLdHL(lin);
    // ld (CURLIN), hl
    context->cpu->addLdiiHL(def_CURLIN);
  }

  for (i = 0; i < t && context->compiled; i++) {
    action = tag->actions[i];

    if (!evalAction(action)) return false;
  }

  return context->compiled;
}

bool CompilerEvaluator::evalActions(ActionNode* action) {
  ActionNode* sub_action;
  unsigned int i, t = action->actions.size();

  for (i = 0; i < t && context->compiled; i++) {
    sub_action = action->actions[i];

    if (!evalAction(sub_action)) return false;
  }

  return context->compiled;
}

bool CompilerEvaluator::evalAction(ActionNode* action) {
  Lexeme* lexeme;
  ICompilerStatementStrategy* statement;

  if (!action) {
    context->syntaxError();
    return false;
  }

  context->current_action = action;
  lexeme = action->lexeme;
  if (!lexeme || lexeme->type != Lexeme::type_keyword) {
    context->syntaxError();
    return false;
  }

  statement = statementFactory->getByKeyword(lexeme->name);
  if (!statement) {
    context->syntaxError();
    return false;
  }

  context->traps_checked = false;
  context->skip_post_trap_check = false;

  if (!statement->execute(context)) {
    if (!context->containErrors()) context->syntaxError();
    return false;
  }

  if (!context->skip_post_trap_check)
    if (!context->traps_checked) {
      context->codeHelper->addCheckTraps();
    }

  return context->compiled;
}
