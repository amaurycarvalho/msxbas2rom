#include "for_statement_strategy.h"

#include "assignment_evaluator.h"
#include "expression_evaluator.h"

bool ForStatementStrategy::parseStatement(ParserContext& context, LexerLine* statement) {
  Lexeme *next_lexeme, *last_lexeme = 0;
  LexerLine parm;
  ActionNode* action;
  int state = 0;
  ExpressionEvaluator exprEval(context);
  AssignmentEvaluator assignEval(context, exprEval);

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("TO")) {
          parm.setLexemeBOF();
          if (!assignEval.evaluate(&parm)) {
            context.error_message = "FOR command without a valid assignment";
            context.eval_expr_error = true;
            return false;
          }

          parm.clearLexemes();

          action = new ActionNode(next_lexeme);
          context.pushActionRoot(action);

          last_lexeme = next_lexeme;

          state = 1;

          continue;
        }

      } break;

      case 1: {
        if (next_lexeme->isKeyword("STEP")) {
          if (last_lexeme->value != "TO") {
            context.error_message = "STEP without a TO clausule";
            context.eval_expr_error = true;
            return false;
          }

          parm.setLexemeBOF();
          if (!evaluateExpression(context, &parm)) {
            context.error_message = "FOR with an invalid TO/STEP";
            context.eval_expr_error = true;
            return false;
          }

          parm.clearLexemes();
          context.popActionRoot();

          action = new ActionNode(next_lexeme);
          context.pushActionRoot(action);

          last_lexeme = next_lexeme;

          state = 2;

          continue;
        }

      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount() && last_lexeme) {
    if (last_lexeme->value != "TO" && last_lexeme->value != "STEP") {
      context.error_message = "FOR command without a TO/STEP complement.";
      context.eval_expr_error = true;
      return false;
    }

    parm.setLexemeBOF();
    if (!evaluateExpression(context, &parm)) {
      context.error_message = "FOR with an invalid TO/STEP";
      context.eval_expr_error = true;
      return false;
    }

    context.popActionRoot();
    parm.clearLexemes();

  } else {
    context.error_message = "Invalid FOR statement (empty)";
    return false;
  }

  return true;
}

bool ForStatementStrategy::execute(ParserContext& context, LexerLine* statement, Lexeme* lexeme) {
  (void)lexeme;
  return parseStatement(context, statement);
}
