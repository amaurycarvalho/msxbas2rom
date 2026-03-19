#include "next_statement_strategy.h"

#include "action_node.h"
#include "lexeme.h"
#include "lexer_line_context.h"

bool NextStatementStrategy::parseStatement(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<Lexeme> next_lexeme;
  shared_ptr<Lexeme> current_lexeme = context->actionRoot->lexeme;
  shared_ptr<ActionNode> action;
  int sepCount = 0;

  while ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->isSeparator("(")) {
      sepCount++;
    } else if (next_lexeme->isSeparator(")")) {
      if (sepCount) sepCount--;
    } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
      context->popActionRoot();
      action = make_shared<ActionNode>(current_lexeme);
      context->pushActionRoot(action);
    }
  }

  return true;
}

bool NextStatementStrategy::execute(shared_ptr<ParserContext> context,
                                    shared_ptr<LexerLineContext> statement,
                                    shared_ptr<Lexeme> lexeme) {
  (void)lexeme;
  return parseStatement(context, statement);
}
