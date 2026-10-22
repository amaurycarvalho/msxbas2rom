#include "for_statement_strategy.h"

#include "action_node.h"
#include "assignment_evaluator.h"
#include "expression_evaluator.h"
#include "lexeme.h"
#include "lexer_line_context.h"
#include "logger.h"

bool ForStatementStrategy::parseStatement(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<Lexeme> next_lexeme, last_lexeme = nullptr;
  shared_ptr<LexerLineContext> parm = make_shared<LexerLineContext>();
  shared_ptr<ActionNode> action;
  int state = 0;

  parm->clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("TO")) {
          parm->setLexemeBOF();
          if (!context->getAssignEval()->evaluate(parm)) {
            context->getLogger()->error("FOR command without a valid assignment");
            context->getEvalExprError() = true;
            return false;
          }

          parm->clearLexemes();

          action = make_shared<ActionNode>(next_lexeme);
          context->pushActionRoot(action);

          last_lexeme = next_lexeme;

          state = 1;

          continue;
        }

      } break;

      case 1: {
        if (next_lexeme->isKeyword("STEP")) {
          if (last_lexeme->value != "TO") {
            context->getLogger()->error("STEP without a TO clausule");
            context->getEvalExprError() = true;
            return false;
          }

          parm->setLexemeBOF();
          if (!evaluateExpression(context, parm)) {
            context->getLogger()->error("FOR with an invalid TO/STEP");
            context->getEvalExprError() = true;
            return false;
          }

          parm->clearLexemes();
          context->popActionRoot();

          action = make_shared<ActionNode>(next_lexeme);
          context->pushActionRoot(action);

          last_lexeme = next_lexeme;

          state = 2;

          continue;
        }

      } break;
    }

    parm->addLexeme(next_lexeme);
  }

  if (parm->getLexemeCount() && last_lexeme) {
    if (last_lexeme->value != "TO" && last_lexeme->value != "STEP") {
      context->getLogger()->error("FOR command without a TO/STEP complement.");
      context->getEvalExprError() = true;
      return false;
    }

    parm->setLexemeBOF();
    if (!evaluateExpression(context, parm)) {
      context->getLogger()->error("FOR with an invalid TO/STEP");
      context->getEvalExprError() = true;
      return false;
    }

    context->popActionRoot();
    parm->clearLexemes();

  } else {
    context->getLogger()->error("Invalid FOR statement (empty)");
    return false;
  }

  return true;
}

bool ForStatementStrategy::execute(shared_ptr<ParserContext> context,
                                   shared_ptr<LexerLineContext> statement,
                                   shared_ptr<Lexeme> lexeme) {
  (void)lexeme;
  if (!context->getAssignEval()) {
    context->getLogger()->error(
        "FOR statement internal error (parser context not initialized)");
    return false;
  }
  return parseStatement(context, statement);
}
