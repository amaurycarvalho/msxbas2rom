#include "if_statement_strategy.h"

#include "assignment_evaluator.h"
#include "expression_evaluator.h"
#include "logger.h"
#include "parser_line_evaluator.h"
#include "parser_statement_strategy_factory.h"

bool IfStatementStrategy::evalPhrase(ParserContext& context,
                                     LexerLine* phrase) {
  ParserStatementStrategyFactory statementStrategyFactory;
  ExpressionEvaluator exprEval(context);
  AssignmentEvaluator assignEval(context, exprEval);
  ParserLineEvaluator lineEval(context, statementStrategyFactory, exprEval,
                               assignEval);
  return lineEval.evaluatePhrase(phrase);
}

bool IfStatementStrategy::parseStatement(ParserContext& context,
                                         LexerLine* statement, int level) {
  shared_ptr<Lexeme> next_lexeme, last_lexeme = statement->getCurrentLexeme();
  LexerLine parm;
  ActionNode* action;
  int state = 0;
  bool testGotoGosub = false, testIf = false, skipEmptyStmtCheck = false;

  action = new ActionNode("COND");
  context.pushActionRoot(action);

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("THEN") || next_lexeme->isKeyword("GOTO") ||
            next_lexeme->isKeyword("GOSUB")) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!evaluateExpression(context, &parm)) {
              context.logger->error("IF command without a valid condition");
              context.eval_expr_error = true;
              return false;
            }
            parm.clearLexemes();
          } else {
            context.logger->error("IF command with a empty condition");
            context.eval_expr_error = true;
            return false;
          }

          context.popActionRoot();

          action = new ActionNode(next_lexeme);
          context.pushActionRoot(action);

          last_lexeme = next_lexeme;

          state = 1;
          testGotoGosub = true;
          testIf = true;

          continue;

        } else if (next_lexeme->isKeyword("ELSE")) {
          context.logger->error("ELSE without a THEN/GOTO/GOSUB");
          context.eval_expr_error = true;
          return false;

        } else if (next_lexeme->isSeparator(":")) {
          context.logger->error("Invalid separator on IF statement");
          context.eval_expr_error = true;
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
              if (!evaluateExpression(context, &parm)) {
                context.logger->error("Invalid expression before IF command");
                context.eval_expr_error = true;
                return false;
              }
              parm.clearLexemes();
            }

            next_lexeme = context.coalesceSymbols(next_lexeme);
            action = new ActionNode(next_lexeme);
            context.pushActionRoot(action);
            if (!parseStatement(context, statement, level + 1)) {
              return false;
            }
            skipEmptyStmtCheck = true;
            testGotoGosub = false;
            continue;
          } else if (next_lexeme->isKeyword("THEN")) {
            context.logger->error("Duplicated THEN in an IF command");
            context.eval_expr_error = true;
            return false;
          }
        }

        if (testGotoGosub) {
          testGotoGosub = false;
          if (next_lexeme->isLiteralNumeric()) {
            if (last_lexeme->isKeyword("THEN") ||
                last_lexeme->isKeyword("ELSE")) {
              action = new ActionNode("GOTO");
              context.pushActionRoot(action);
              context.pushActionFromLexeme(next_lexeme);
              context.popActionRoot();
            } else if (last_lexeme->isKeyword("GOTO")) {
              if (state == 1)
                last_lexeme->value = "THEN";
              else
                last_lexeme->value = "ELSE";
              last_lexeme->name = last_lexeme->value;
              action = new ActionNode("GOTO");
              context.pushActionRoot(action);
              context.pushActionFromLexeme(next_lexeme);
              context.popActionRoot();
            } else if (last_lexeme->isKeyword("GOSUB")) {
              if (state == 1)
                last_lexeme->value = "THEN";
              else
                last_lexeme->value = "ELSE";
              last_lexeme->name = last_lexeme->value;
              action = new ActionNode("GOSUB");
              context.pushActionRoot(action);
              context.pushActionFromLexeme(next_lexeme);
              context.popActionRoot();
            } else {
              context.logger->error("IF with invalid GOTO/GOSUB parameter");
              context.eval_expr_error = true;
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
            if (!this->evalPhrase(context, &parm)) return false;
            parm.clearLexemes();
          } else {
            if (skipEmptyStmtCheck) {
              skipEmptyStmtCheck = false;
            } else {
              context.logger->error("IF with an empty statement");
              context.eval_expr_error = true;
              return false;
            }
          }

          if (next_lexeme->isKeyword("ELSE")) {
            context.popActionRoot();

            if (state == 1) {
              action = new ActionNode(next_lexeme);
              context.pushActionRoot(action);
              last_lexeme = next_lexeme;
              testGotoGosub = true;
              state = 2;
            } else {
              if (level) {
                context.popActionRoot();
                statement->getPreviousLexeme();
                return true;
              }
              context.logger->error("Duplicated ELSE in an IF command");
              context.eval_expr_error = true;
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
    context.logger->error("IF without a THEN/GOTO/GOSUB/ELSE complement");
    context.eval_expr_error = true;
    return false;
  }

  if (parm.getLexemeCount()) {
    if (state > 2) {
      context.logger->error("Code detected after end of an IF statement");
      context.eval_expr_error = true;
      return false;
    }

    if (testGotoGosub) {
      next_lexeme = parm.getFirstLexeme();
      if (next_lexeme->isLiteralNumeric()) {
        action = new ActionNode("GOTO");
        context.pushActionRoot(action);
        context.pushActionFromLexeme(next_lexeme);
        context.popActionRoot();

        parm.clearLexemes();
      }
    } else {
      if (parm.getLexemeCount()) {
        parm.setLexemeBOF();
        if (!this->evalPhrase(context, &parm)) return false;
        parm.clearLexemes();
      }
    }

    context.popActionRoot();
  }

  context.popActionRoot();

  return true;
}

bool IfStatementStrategy::execute(ParserContext& context, LexerLine* statement,
                                  shared_ptr<Lexeme> lexeme) {
  (void)lexeme;
  return parseStatement(context, statement, 0);
}
