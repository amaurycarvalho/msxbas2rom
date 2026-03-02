#include "for_statement_strategy.h"

#include "parser.h"

bool ForStatementStrategy::parseStatement(Parser& parser, LexerLine* statement) {
  Lexeme *next_lexeme, *last_lexeme = 0;
  LexerLine parm;
  ActionNode* action;
  int state = 0;
  ParserContext& ctx = parser.getContext();

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("TO")) {
          parm.setLexemeBOF();
          if (!parser.evalAssignmentTokens(&parm)) {
            ctx.error_message = "FOR command without a valid assignment";
            ctx.eval_expr_error = true;
            return false;
          }

          parm.clearLexemes();

          action = new ActionNode(next_lexeme);
          parser.pushActionNodeRoot(action);

          last_lexeme = next_lexeme;

          state = 1;

          continue;
        }

      } break;

      case 1: {
        if (next_lexeme->isKeyword("STEP")) {
          if (last_lexeme->value != "TO") {
            ctx.error_message = "STEP without a TO clausule";
            ctx.eval_expr_error = true;
            return false;
          }

          parm.setLexemeBOF();
          if (!parser.evalExpressionTokens(&parm)) {
            ctx.error_message = "FOR with an invalid TO/STEP";
            ctx.eval_expr_error = true;
            return false;
          }

          parm.clearLexemes();
          parser.popActionNodeRoot();

          action = new ActionNode(next_lexeme);
          parser.pushActionNodeRoot(action);

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
      ctx.error_message = "FOR command without a TO/STEP complement.";
      ctx.eval_expr_error = true;
      return false;
    }

    parm.setLexemeBOF();
    if (!parser.evalExpressionTokens(&parm)) {
      ctx.error_message = "FOR with an invalid TO/STEP";
      ctx.eval_expr_error = true;
      return false;
    }

    parser.popActionNodeRoot();
    parm.clearLexemes();

  } else {
    ctx.error_message = "Invalid FOR statement (empty)";
    return false;
  }

  return true;
}

bool ForStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                   Lexeme* lexeme) {
  (void)lexeme;
  return parseStatement(parser, statement);
}
