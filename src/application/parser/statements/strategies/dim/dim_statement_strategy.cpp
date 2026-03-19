#include "dim_statement_strategy.h"

#include "action_node.h"
#include "generic_statement_strategy.h"
#include "lexeme.h"
#include "logger.h"

bool DimStatementStrategy::parseStatement(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<ActionNode> action = context->actionRoot;
  shared_ptr<ActionNode> subaction;
  shared_ptr<Lexeme> lexeme;
  unsigned int i, t;

  GenericStatementStrategy genericStrategy;
  if (!genericStrategy.parseStatement(context, statement)) return false;

  t = action->actions.size();
  if (!t) return false;

  for (i = 0; i < t; i++) {
    subaction = action->actions[i];
    lexeme = subaction->lexeme;
    lexeme->isArray = true;
    lexeme->parm_count = subaction->actions.size();
    if (!lexeme->parm_count) {
      context->logger->error(
          "Invalid array declaration: DIM size parameter is missing");
      return false;
    }
  }

  return true;
}

bool DimStatementStrategy::execute(shared_ptr<ParserContext> context,
                                   shared_ptr<LexerLineContext> statement,
                                   shared_ptr<Lexeme> lexeme) {
  (void)lexeme;
  return parseStatement(context, statement);
}
