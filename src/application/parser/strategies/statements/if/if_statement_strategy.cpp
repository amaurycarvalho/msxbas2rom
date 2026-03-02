#include "if_statement_strategy.h"

#include "parser.h"

bool IfStatementStrategy::parseStatement(Parser& parser, LexerLine* statement,
                                         int level) {
  Lexeme *next_lexeme, *last_lexeme = statement->getCurrentLexeme();
  LexerLine parm;
  ActionNode* action;
  int state = 0;
  bool testGotoGosub = false, testIf = false, skipEmptyStmtCheck = false;
  ParserContext& ctx = parser.getContext();

  action = new ActionNode("COND");
  parser.pushActionNodeRoot(action);

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("THEN") || next_lexeme->isKeyword("GOTO") ||
            next_lexeme->isKeyword("GOSUB")) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!parser.evalExpressionTokens(&parm)) {
              ctx.error_message = "IF command without a valid condition";
              ctx.eval_expr_error = true;
              return false;
            }
            parm.clearLexemes();
          } else {
            ctx.error_message = "IF command with a empty condition";
            ctx.eval_expr_error = true;
            return false;
          }

          parser.popActionNodeRoot();

          action = new ActionNode(next_lexeme);
          parser.pushActionNodeRoot(action);

          last_lexeme = next_lexeme;

          state = 1;
          testGotoGosub = true;
          testIf = true;

          continue;

        } else if (next_lexeme->isKeyword("ELSE")) {
          ctx.error_message = "ELSE without a THEN/GOTO/GOSUB";
          ctx.eval_expr_error = true;
          return false;

        } else if (next_lexeme->isSeparator(":")) {
          ctx.error_message = "Invalid separator on IF statement";
          ctx.eval_expr_error = true;
          return false;
        }

      } break;

      case 1:
      case 2: {
        if (testIf) {
          testIf = false;
          if (next_lexeme->isKeyword("IF")) {
            if (parm.getLexemeCount()) {
              parm.setLexemeBOF();
              if (!parser.evalExpressionTokens(&parm)) {
                ctx.error_message = "Invalid expression before IF command";
                ctx.eval_expr_error = true;
                return false;
              }
              parm.clearLexemes();
            }

            next_lexeme = parser.coalesceLexeme(next_lexeme);
            action = new ActionNode(next_lexeme);
            parser.pushActionNodeRoot(action);
            if (!parseStatement(parser, statement, level + 1)) {
              return false;
            }
            skipEmptyStmtCheck = true;
            testGotoGosub = false;
            continue;
          } else if (next_lexeme->isKeyword("THEN")) {
            ctx.error_message = "Duplicated THEN in an IF command";
            ctx.eval_expr_error = true;
            return false;
          }
        }

        if (testGotoGosub) {
          testGotoGosub = false;
          if (next_lexeme->isLiteralNumeric()) {
            if (last_lexeme->isKeyword("THEN") ||
                last_lexeme->isKeyword("ELSE")) {
              action = new ActionNode("GOTO");
              parser.pushActionNodeRoot(action);
              parser.pushActionFromLexemeNode(next_lexeme);
              parser.popActionNodeRoot();
            } else if (last_lexeme->isKeyword("GOTO")) {
              if (state == 1)
                last_lexeme->value = "THEN";
              else
                last_lexeme->value = "ELSE";
              last_lexeme->name = last_lexeme->value;
              action = new ActionNode("GOTO");
              parser.pushActionNodeRoot(action);
              parser.pushActionFromLexemeNode(next_lexeme);
              parser.popActionNodeRoot();
            } else if (last_lexeme->isKeyword("GOSUB")) {
              if (state == 1)
                last_lexeme->value = "THEN";
              else
                last_lexeme->value = "ELSE";
              last_lexeme->name = last_lexeme->value;
              action = new ActionNode("GOSUB");
              parser.pushActionNodeRoot(action);
              parser.pushActionFromLexemeNode(next_lexeme);
              parser.popActionNodeRoot();
            } else {
              ctx.error_message = "IF with invalid GOTO/GOSUB parameter";
              ctx.eval_expr_error = true;
              return false;
            }

            parm.clearLexemes();
            skipEmptyStmtCheck = true;

            continue;
          }
        }

        if (next_lexeme->isSeparator(":") || next_lexeme->isKeyword("ELSE")) {
          testIf = true;

          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!parser.evalPhraseTokens(&parm)) return false;
            parm.clearLexemes();
          } else {
            if (skipEmptyStmtCheck) {
              skipEmptyStmtCheck = false;
            } else {
              ctx.error_message = "IF with an empty statement";
              ctx.eval_expr_error = true;
              return false;
            }
          }

          if (next_lexeme->isKeyword("ELSE")) {
            parser.popActionNodeRoot();

            if (state == 1) {
              action = new ActionNode(next_lexeme);
              parser.pushActionNodeRoot(action);
              last_lexeme = next_lexeme;
              testGotoGosub = true;
              state = 2;
            } else {
              if (level) {
                parser.popActionNodeRoot();
                statement->getPreviousLexeme();
                return true;
              }
              ctx.error_message = "Duplicated ELSE in an IF command";
              ctx.eval_expr_error = true;
              return false;
            }
          }

          continue;
        }

      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (state == 0) {
    ctx.error_message = "IF without a THEN/GOTO/GOSUB/ELSE complement";
    ctx.eval_expr_error = true;
    return false;
  }

  if (parm.getLexemeCount()) {
    if (state > 2) {
      ctx.error_message = "Code detected after end of an IF statement";
      ctx.eval_expr_error = true;
      return false;
    }

    if (testGotoGosub) {
      next_lexeme = parm.getFirstLexeme();
      if (next_lexeme->isLiteralNumeric()) {
        action = new ActionNode("GOTO");
        parser.pushActionNodeRoot(action);
        parser.pushActionFromLexemeNode(next_lexeme);
        parser.popActionNodeRoot();

        parm.clearLexemes();
      }
    } else {
      if (parm.getLexemeCount()) {
        parm.setLexemeBOF();
        if (!parser.evalPhraseTokens(&parm)) return false;
        parm.clearLexemes();
      }
    }

    parser.popActionNodeRoot();
  }

  parser.popActionNodeRoot();

  return true;
}

bool IfStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                  Lexeme* lexeme) {
  (void)lexeme;
  return parseStatement(parser, statement, 0);
}
