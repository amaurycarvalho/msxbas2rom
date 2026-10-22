/***
 * @file compiler_evaluator.cpp
 * @brief Compiler evaluator
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_evaluator.h"

#include "action_node.h"
#include "build_options.h"
#include "compiler_code_helper.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "compiler_statement_strategy.h"
#include "compiler_statement_strategy_factory.h"
#include "lexeme.h"
#include "logger.h"
#include "tag_node.h"

CompilerEvaluator::CompilerEvaluator(shared_ptr<CompilerContext> context) {
  this->context = context;
  statementFactory = make_shared<CompilerStatementStrategyFactory>();
}

CompilerEvaluator::~CompilerEvaluator() = default;

bool CompilerEvaluator::evaluate(shared_ptr<TagNode> tag) {
  shared_ptr<ActionNode> action;
  unsigned int i, t = tag->actions.size(), lin;

  context->getCurrentTag() = tag;

  if (context->getOpts()->lineNumber) {
    try {
      lin = stoi(tag->name);
    } catch (exception& e) {
      context->getLogger()->warning("Error while converting numeric constant " +
                               tag->name);
      lin = 0;
    }
    // ld hl, line number
    context->getCpu()->addLdHL(lin);
    // ld (CURLIN), hl
    context->getCpu()->addLdiiHL(def_CURLIN);
  }

  for (i = 0; i < t && context->getCompiled(); i++) {
    action = tag->actions[i];

    if (!evalAction(action)) return false;
  }

  return context->getCompiled();
}

bool CompilerEvaluator::evalActions(shared_ptr<ActionNode> action) {
  shared_ptr<ActionNode> sub_action;
  unsigned int i, t = action->actions.size();

  for (i = 0; i < t && context->getCompiled(); i++) {
    sub_action = action->actions[i];

    if (!evalAction(sub_action)) return false;
  }

  return context->getCompiled();
}

bool CompilerEvaluator::evalAction(shared_ptr<ActionNode> action) {
  shared_ptr<Lexeme> lexeme;
  ICompilerStatementStrategy* statement;

  if (!action) {
    context->syntaxError();
    return false;
  }

  context->getCurrentAction() = action;
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

  context->getTrapsChecked() = false;
  context->getSkipPostTrapCheck() = false;

  if (!statement->execute(context)) {
    if (!context->containErrors()) context->syntaxError();
    return false;
  }

  if (!context->getSkipPostTrapCheck())
    if (!context->getTrapsChecked()) {
      context->getCodeHelper()->addCheckTraps();
    }

  return context->getCompiled();
}
