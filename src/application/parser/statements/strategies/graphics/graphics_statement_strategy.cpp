#include "graphics_statement_strategy.h"

#include "action_node.h"
#include "generic_statement_strategy.h"
#include "lexeme.h"
#include "lexer_line_context.h"
#include "logger.h"
#include "print_statement_strategy.h"

bool GraphicsStatementStrategy::parsePset(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<Lexeme> next_lexeme;
  shared_ptr<ActionNode> action;
  shared_ptr<LexerLineContext> parm = make_shared<LexerLineContext>();
  int state = 0, sepCount = 0, parmCount = 0;
  bool mustPopAction = false, isKeyword = false;
  string parmValue;

  parm->clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = context->coalesceSymbols(next_lexeme);

    if (state == 0) {
      if (next_lexeme->isKeyword("STEP")) {
        action = make_shared<ActionNode>(next_lexeme);
        context->pushActionRoot(action);
        continue;
      } else if (next_lexeme->isSeparator("(")) {
        state++;
        parmCount++;
        if (context->getActionRoot()->lexeme->value != "STEP") {
          action = make_shared<ActionNode>("COORD");
          context->pushActionRoot(action);
        }
        continue;
      } else {
        context->getLogger()->error("PSET without a valid complement");
        context->getEvalExprError() = true;
        return false;
      }
    } else if (state == 1) {
      if (next_lexeme->isSeparator("(")) {
        sepCount++;
      } else if (next_lexeme->isSeparator(")")) {
        if (sepCount) {
          sepCount--;
        } else {
          mustPopAction = true;
          parmCount++;
          continue;
        }
      } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
        if (parm->getLexemeCount()) {
          parm->setLexemeBOF();
          if (!evaluateExpression(context, parm)) {
            return false;
          }
          parm->clearLexemes();
          if (mustPopAction) {
            context->popActionRoot();
            mustPopAction = false;
          } else
            parmCount++;
        } else {
          parmCount++;
          next_lexeme = context->getLexNull();
          context->pushActionFromLexeme(next_lexeme);
        }
        continue;
      }
      parm->addLexeme(next_lexeme);
    }
  }

  if (parm->getLexemeCount()) {
    next_lexeme = parm->getFirstLexeme();

    if (parmCount > 3) {
      if (next_lexeme->type == Lexeme::type_keyword ||
          next_lexeme->type == Lexeme::type_operator) {
        int operatorCode = context->gfxOperatorCode(next_lexeme);

        isKeyword = true;

        if (operatorCode >= 0) {
          parmValue = to_string(operatorCode);
        } else {
          context->getLogger()->error(
              "Invalid operator parameter in PSET statement");
          context->getEvalExprError() = true;
          return false;
        }
      }
    }

    if (isKeyword) {
      next_lexeme = make_shared<Lexeme>(Lexeme::type_literal,
                                        Lexeme::subtype_numeric, parmValue);
      context->pushActionFromLexeme(next_lexeme);
    } else {
      parm->setLexemeBOF();
      if (!evaluateExpression(context, parm)) {
        return false;
      }
    }

    parm->clearLexemes();
  }

  return true;
}

bool GraphicsStatementStrategy::parseLine(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<Lexeme> next_lexeme;
  shared_ptr<ActionNode> action;
  shared_ptr<LexerLineContext> parm = make_shared<LexerLineContext>();
  int state = 0, sepCount = 0, parmCount = 0;
  bool startAsParm2 = false, mustPopAction = false;
  bool sepTime = false, isSpecialParameter = false;
  string parmValue;

  parm->clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = context->coalesceSymbols(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("INPUT")) {
          context->getHasInput() = true;
          context->pushActionFromLexeme(next_lexeme);
          PrintStatementStrategy printStrategy;
          return printStrategy.parseStatement(context, statement);
        } else if (next_lexeme->isKeyword("STEP")) {
          if (startAsParm2) {
            action = make_shared<ActionNode>("TO_STEP");
            context->pushActionRoot(action);
          } else {
            action = make_shared<ActionNode>(next_lexeme);
            context->pushActionRoot(action);
          }
          continue;
        } else if (next_lexeme->isSeparator("(")) {
          parmCount++;
          if (startAsParm2)
            state = 3;
          else
            state++;
          if (context->getActionRoot()->lexeme->value != "STEP" &&
              context->getActionRoot()->lexeme->value != "TO_STEP") {
            action = make_shared<ActionNode>("COORD");
            if (startAsParm2) action->lexeme->name = "TO_COORD";
            action->lexeme->value = action->lexeme->name;
            context->pushActionRoot(action);
          }
          continue;
        } else if (next_lexeme->isOperator("-")) {
          startAsParm2 = true;
          continue;
        } else {
          context->getLogger()->error("LINE without a valid complement");
          context->getEvalExprError() = true;
          return false;
        }
      } break;

      case 1: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount)
            sepCount--;
          else {
            if (sepTime) {
              context->getLogger()->error(
                  "Invalid parentheses syntax in LINE statement");
              context->getEvalExprError() = true;
              return false;
            }
            if (parm->getLexemeCount()) {
              parm->setLexemeBOF();
              if (!evaluateExpression(context, parm)) {
                return false;
              }
              parm->clearLexemes();
              context->popActionRoot();
            }
            sepTime = true;
            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm->getLexemeCount()) {
            parm->setLexemeBOF();
            if (!evaluateExpression(context, parm)) {
              return false;
            }
            parm->clearLexemes();
          }
          continue;
        } else if (next_lexeme->isOperator("-") && sepTime) {
          state++;
          continue;
        }

      } break;

      case 2: {
        if (next_lexeme->isKeyword("STEP")) {
          action = make_shared<ActionNode>("TO_STEP");
          context->pushActionRoot(action);
          continue;
        } else if (next_lexeme->isSeparator("(")) {
          state++;
          if (context->getActionRoot()) {
            if (context->getActionRoot()->lexeme->value != "TO_STEP") {
              action = make_shared<ActionNode>("TO_COORD");
              context->pushActionRoot(action);
            }
          } else {
            context->getLogger()->error(
                "Invalid parentheses syntax in LINE statement");
            context->getEvalExprError() = true;
            return false;
          }
          continue;
        }

      } break;

      case 3: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount)
            sepCount--;
          else {
            if (parmCount != 2 || mustPopAction) {
              context->getLogger()->error(
                  "Invalid parentheses syntax in LINE statement");
              context->getEvalExprError() = true;
              return false;
            }
            mustPopAction = true;
            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          parmCount++;

          if (parm->getLexemeCount()) {
            next_lexeme = parm->getFirstLexeme();

            isSpecialParameter = false;

            if (parmCount >= 3 && !mustPopAction) {
              if (next_lexeme->type == Lexeme::type_identifier) {
                if (next_lexeme->value == "B") {
                  parmValue = "1";
                  isSpecialParameter = true;
                } else if (next_lexeme->value == "BF") {
                  parmValue = "2";
                  isSpecialParameter = true;
                }
              }
            }

            if (isSpecialParameter) {
              next_lexeme = make_shared<Lexeme>(
                  Lexeme::type_literal, Lexeme::subtype_numeric, parmValue);
              context->pushActionFromLexeme(next_lexeme);
            } else {
              parm->setLexemeBOF();
              if (!evaluateExpression(context, parm)) {
                return false;
              }
              if (mustPopAction) {
                context->popActionRoot();
                mustPopAction = false;
              }
            }

            parm->clearLexemes();

          } else {
            next_lexeme = context->getLexNull();
            context->pushActionFromLexeme(next_lexeme);
          }

          continue;

        } else if (mustPopAction) {
          context->getLogger()->error("Invalid syntax in LINE statement");
          context->getEvalExprError() = true;
          return false;
        }

      } break;
    }

    parm->addLexeme(next_lexeme);
  }

  if (parm->getLexemeCount()) {
    next_lexeme = parm->getFirstLexeme();

    isSpecialParameter = false;

    if (parmCount >= 3) {
      if (next_lexeme->type == Lexeme::type_identifier) {
        if (next_lexeme->value == "B") {
          parmValue = "1";
          isSpecialParameter = true;
        } else if (next_lexeme->value == "BF") {
          parmValue = "2";
          isSpecialParameter = true;
        }
      } else if (parmCount > 3) {
        if (next_lexeme->type == Lexeme::type_keyword ||
            (next_lexeme->type == Lexeme::type_operator &&
             next_lexeme->value != "+" && next_lexeme->value != "-")) {
          int operatorCode = context->gfxOperatorCode(next_lexeme);

          isSpecialParameter = true;

          if (operatorCode >= 0) {
            parmValue = to_string(operatorCode);
          } else {
            context->getLogger()->error(
                "Invalid operator parameter in LINE statement.");
            context->getEvalExprError() = true;
            return false;
          }
        }
      }
    }

    if (isSpecialParameter) {
      next_lexeme = make_shared<Lexeme>(Lexeme::type_literal,
                                        Lexeme::subtype_numeric, parmValue);
      context->pushActionFromLexeme(next_lexeme);
    } else {
      parm->setLexemeBOF();
      if (!evaluateExpression(context, parm)) {
        return false;
      }
    }

    parm->clearLexemes();
  }

  return true;
}

bool GraphicsStatementStrategy::parseCircle(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<Lexeme> next_lexeme;
  shared_ptr<ActionNode> action;
  shared_ptr<LexerLineContext> parm = make_shared<LexerLineContext>();
  int state = 0, sepCount = 0, parmCount = 0;
  bool mustPopAction = false;

  parm->clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = context->coalesceSymbols(next_lexeme);

    if (state == 0) {
      if (next_lexeme->isKeyword("STEP")) {
        action = make_shared<ActionNode>(next_lexeme);
        context->pushActionRoot(action);
        continue;
      } else if (next_lexeme->isSeparator("(")) {
        state++;
        parmCount++;
        if (context->getActionRoot()->lexeme->value != "STEP") {
          action = make_shared<ActionNode>("COORD");
          context->pushActionRoot(action);
        }
        continue;
      } else {
        context->getLogger()->error("CIRCLE without a valid complement");
        context->getEvalExprError() = true;
        return false;
      }
    } else if (state == 1) {
      if (next_lexeme->isSeparator("(")) {
        sepCount++;
      } else if (next_lexeme->isSeparator(")")) {
        if (sepCount) {
          sepCount--;
        } else {
          mustPopAction = true;
          parmCount++;
          continue;
        }
      } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
        if (parm->getLexemeCount()) {
          parm->setLexemeBOF();
          if (!evaluateExpression(context, parm)) {
            return false;
          }
          parm->clearLexemes();
          if (mustPopAction) {
            context->popActionRoot();
            mustPopAction = false;
          } else
            parmCount++;
        } else {
          parmCount++;
          next_lexeme = context->getLexNull();
          context->pushActionFromLexeme(next_lexeme);
        }
        continue;
      }
      parm->addLexeme(next_lexeme);
    }
  }

  if (parm->getLexemeCount()) {
    parm->setLexemeBOF();
    if (!evaluateExpression(context, parm)) {
      return false;
    }

    parm->clearLexemes();
  }

  return true;
}

bool GraphicsStatementStrategy::parsePaint(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<Lexeme> next_lexeme;
  shared_ptr<ActionNode> action;
  shared_ptr<LexerLineContext> parm = make_shared<LexerLineContext>();
  int state = 0, sepCount = 0, parmCount = 0;
  bool mustPopAction = false;

  parm->clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = context->coalesceSymbols(next_lexeme);

    if (state == 0) {
      if (next_lexeme->isKeyword("STEP")) {
        action = make_shared<ActionNode>(next_lexeme);
        context->pushActionRoot(action);
        continue;
      } else if (next_lexeme->isSeparator("(")) {
        state++;
        parmCount++;
        if (context->getActionRoot()->lexeme->value != "STEP") {
          action = make_shared<ActionNode>("COORD");
          context->pushActionRoot(action);
        }
        continue;
      } else {
        context->getLogger()->error("PAINT without a valid complement.");
        context->getEvalExprError() = true;
        return false;
      }
    } else if (state == 1) {
      if (next_lexeme->isSeparator("(")) {
        sepCount++;
      } else if (next_lexeme->isSeparator(")")) {
        if (sepCount) {
          sepCount--;
        } else {
          mustPopAction = true;
          parmCount++;
          continue;
        }
      } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
        if (parm->getLexemeCount()) {
          parm->setLexemeBOF();
          if (!evaluateExpression(context, parm)) {
            return false;
          }
          parm->clearLexemes();
          if (mustPopAction) {
            context->popActionRoot();
            mustPopAction = false;
          } else
            parmCount++;
        } else {
          next_lexeme = context->getLexNull();
          context->pushActionFromLexeme(next_lexeme);
        }
        continue;
      }
      parm->addLexeme(next_lexeme);
    }
  }

  if (parm->getLexemeCount()) {
    parm->setLexemeBOF();
    if (!evaluateExpression(context, parm)) {
      return false;
    }

    parm->clearLexemes();
  }

  return true;
}

bool GraphicsStatementStrategy::parseCopy(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<Lexeme> next_lexeme;
  shared_ptr<ActionNode> action;
  shared_ptr<LexerLineContext> parm = make_shared<LexerLineContext>();
  int state = 0, sepCount = 0, parmCount = 0;
  bool isKeyword = false;
  string parmValue;

  parm->clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = context->coalesceSymbols(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("SCREEN")) {
          context->pushActionFromLexeme(next_lexeme);
          GenericStatementStrategy genericStrategy;
          return genericStrategy.parseStatement(context, statement);
        }
        if (next_lexeme->isSeparator("(")) {
          parmCount++;
          state = 1;

          action = make_shared<ActionNode>("COORD");
          context->pushActionRoot(action);

          continue;

        } else {
          state = 9;
        }
      } break;

      case 1: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount)
            sepCount--;
          else {
            if (parm->getLexemeCount()) {
              parm->setLexemeBOF();
              if (!evaluateExpression(context, parm)) {
                return false;
              }
              parm->clearLexemes();
            } else {
              next_lexeme = context->getLexNull();
              context->pushActionFromLexeme(next_lexeme);
            }
            context->popActionRoot();
            state = 10;
            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm->getLexemeCount()) {
            parm->setLexemeBOF();
            if (!evaluateExpression(context, parm)) {
              return false;
            }
            parm->clearLexemes();
          } else {
            next_lexeme = context->getLexNull();
            context->pushActionFromLexeme(next_lexeme);
          }
          continue;
        }

      } break;

      case 2: {
        if (next_lexeme->isKeyword("STEP")) {
          action = make_shared<ActionNode>("TO_STEP");
          context->pushActionRoot(action);
          continue;
        } else if (next_lexeme->isSeparator("(")) {
          parmCount++;
          state = 3;
          if (context->getActionRoot()->lexeme->value != "TO_STEP") {
            action = make_shared<ActionNode>("TO_COORD");
            context->pushActionRoot(action);
          }
          continue;
        }

      } break;

      case 3: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount)
            sepCount--;
          else {
            if (parm->getLexemeCount()) {
              parm->setLexemeBOF();
              if (!evaluateExpression(context, parm)) {
                return false;
              }
              parm->clearLexemes();
            } else {
              next_lexeme = context->getLexNull();
              context->pushActionFromLexeme(next_lexeme);
            }

            context->popActionRoot();

            parmCount++;
            state = 4;

            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm->getLexemeCount()) {
            parm->setLexemeBOF();
            if (!evaluateExpression(context, parm)) {
              return false;
            }
            parm->clearLexemes();
          } else {
            next_lexeme = context->getLexNull();
            context->pushActionFromLexeme(next_lexeme);
          }

          continue;
        }

      } break;

      case 4: {
        if (next_lexeme->isKeyword("TO")) {
          if (parm->getLexemeCount()) {
            parm->setLexemeBOF();
            if (!evaluateExpression(context, parm)) {
              return false;
            }
            parm->clearLexemes();
          } else {
            next_lexeme = context->getLexNull();
            context->pushActionFromLexeme(next_lexeme);
          }

          parmCount++;
          state = 5;

          continue;

        } else if (next_lexeme->isSeparator(",")) {
          continue;
        }

      } break;

      case 5: {
        parmCount++;
        if (next_lexeme->isSeparator("(")) {
          state = 6;

          action = make_shared<ActionNode>("TO_DEST");
          context->pushActionRoot(action);

          continue;
        } else {
          state = 8;
        }

      } break;

      case 6: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount)
            sepCount--;
          else {
            if (parm->getLexemeCount()) {
              parm->setLexemeBOF();
              if (!evaluateExpression(context, parm)) {
                return false;
              }
              parm->clearLexemes();
            } else {
              next_lexeme = context->getLexNull();
              context->pushActionFromLexeme(next_lexeme);
            }

            context->popActionRoot();

            state = 7;

            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm->getLexemeCount()) {
            parm->setLexemeBOF();
            if (!evaluateExpression(context, parm)) {
              return false;
            }
            parm->clearLexemes();
          } else {
            next_lexeme = context->getLexNull();
            context->pushActionFromLexeme(next_lexeme);
          }

          continue;
        }

      } break;

      case 7: {
        if (next_lexeme->isSeparator(",")) {
          state = 8;

          continue;
        }

      } break;

      case 8: {
        if (next_lexeme->isSeparator(",")) {
          if (parm->getLexemeCount()) {
            parm->setLexemeBOF();
            if (!evaluateExpression(context, parm)) {
              return false;
            }
            parm->clearLexemes();
          } else {
            next_lexeme = context->getLexNull();
            context->pushActionFromLexeme(next_lexeme);
          }

          parmCount++;

          continue;
        }

      } break;

      case 9: {
        if (next_lexeme->isKeyword("TO")) {
          if (parm->getLexemeCount()) {
            parm->setLexemeBOF();
            if (!evaluateExpression(context, parm)) {
              return false;
            }
            parm->clearLexemes();
          } else {
            next_lexeme = context->getLexNull();
            context->pushActionFromLexeme(next_lexeme);
          }

          if (parmCount == 0) {
            context->pushActionFromLexeme(context->getLexNull());
          }

          parmCount++;
          state = 5;

          continue;

        } else if (next_lexeme->isSeparator(",")) {
          if (parm->getLexemeCount()) {
            parm->setLexemeBOF();
            if (!evaluateExpression(context, parm)) {
              return false;
            }
            parm->clearLexemes();
          } else {
            next_lexeme = context->getLexNull();
            context->pushActionFromLexeme(next_lexeme);
          }

          continue;
        }

      } break;

      case 10: {
        if (next_lexeme->isOperator("-")) {
          state = 2;
          continue;
        } else {
          context->getLogger()->error("Invalid syntax on COPY statement");
          return false;
        }
      } break;
    }

    parm->addLexeme(next_lexeme);
  }

  if (parm->getLexemeCount()) {
    next_lexeme = parm->getFirstLexeme();

    if (parmCount >= 5) {
      if ((next_lexeme->type == Lexeme::type_keyword &&
           next_lexeme->subtype != Lexeme::subtype_function) ||
          next_lexeme->type == Lexeme::type_operator) {
        int operatorCode = context->gfxOperatorCode(next_lexeme);

        isKeyword = true;

        if (operatorCode >= 0) {
          parmValue = to_string(operatorCode);
        } else {
          context->getLogger()->error(
              "Invalid operator parameter in COPY statement");
          context->getEvalExprError() = true;
          return false;
        }
      }
    }

    if (isKeyword) {
      next_lexeme = make_shared<Lexeme>(Lexeme::type_literal,
                                        Lexeme::subtype_numeric, parmValue);
      context->pushActionFromLexeme(next_lexeme);
    } else {
      parm->setLexemeBOF();
      if (!evaluateExpression(context, parm)) {
        return false;
      }
    }

    parm->clearLexemes();
  }

  return true;
}

bool GraphicsStatementStrategy::execute(shared_ptr<ParserContext> context,
                                        shared_ptr<LexerLineContext> statement,
                                        shared_ptr<Lexeme> lexeme) {
  if (lexeme->value == "PSET" || lexeme->value == "PRESET")
    return parsePset(context, statement);
  if (lexeme->value == "LINE") return parseLine(context, statement);
  if (lexeme->value == "CIRCLE") return parseCircle(context, statement);
  if (lexeme->value == "PAINT") return parsePaint(context, statement);
  return parseCopy(context, statement);
}
