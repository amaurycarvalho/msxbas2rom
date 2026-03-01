#include "graphics_statement_strategy.h"

#include "generic_statement_strategy.h"
#include "parser.h"
#include "print_statement_strategy.h"

bool GraphicsStatementStrategy::parsePset(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  LexerLine parm;
  int state = 0, sepCount = 0, parmCount = 0;
  bool mustPopAction = false, isKeyword = false;
  string parmValue;
  ParserContext& ctx = parser.getContext();

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = parser.coalesceLexeme(next_lexeme);

    if (state == 0) {
      if (next_lexeme->isKeyword("STEP")) {
        action = new ActionNode(next_lexeme);
        parser.pushActionNodeRoot(action);
        continue;
      } else if (next_lexeme->isSeparator("(")) {
        state++;
        parmCount++;
        if (ctx.actionRoot->lexeme->value != "STEP") {
          action = new ActionNode("COORD");
          parser.pushActionNodeRoot(action);
        }
        continue;
      } else {
        ctx.error_message = "PSET without a valid complement.";
        ctx.eval_expr_error = true;
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
        if (parm.getLexemeCount()) {
          parm.setLexemeBOF();
          if (!parser.evalExpressionTokens(&parm)) {
            return false;
          }
          parm.clearLexemes();
          if (mustPopAction) {
            parser.popActionNodeRoot();
            mustPopAction = false;
          } else
            parmCount++;
        } else {
          parmCount++;
          next_lexeme = ctx.lex_null;
          parser.pushActionFromLexemeNode(next_lexeme);
        }
        continue;
      }
      parm.addLexeme(next_lexeme);
    }
  }

  if (parm.getLexemeCount()) {
    next_lexeme = parm.getFirstLexeme();

    if (parmCount > 3) {
      if (next_lexeme->type == Lexeme::type_keyword ||
          next_lexeme->type == Lexeme::type_operator) {
        int operatorCode = parser.gfxOperatorFromLexeme(next_lexeme);

        isKeyword = true;

        if (operatorCode >= 0) {
          parmValue = to_string(operatorCode);
        } else {
          ctx.error_message = "Invalid operator parameter in PSET statement.";
          ctx.eval_expr_error = true;
          return false;
        }
      }
    }

    if (isKeyword) {
      next_lexeme =
          new Lexeme(Lexeme::type_literal, Lexeme::subtype_numeric, parmValue);
      parser.pushActionFromLexemeNode(next_lexeme);
    } else {
      parm.setLexemeBOF();
      if (!parser.evalExpressionTokens(&parm)) {
        return false;
      }
    }

    parm.clearLexemes();
  }

  return true;
}

bool GraphicsStatementStrategy::parseLine(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  LexerLine parm;
  int state = 0, sepCount = 0, parmCount = 0;
  bool startAsParm2 = false, mustPopAction = false;
  bool sepTime = false, isSpecialParameter = false;
  string parmValue;
  ParserContext& ctx = parser.getContext();

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = parser.coalesceLexeme(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("INPUT")) {
          ctx.has_input = true;
          parser.pushActionFromLexemeNode(next_lexeme);
          PrintStatementStrategy printStrategy;
          return printStrategy.parseStatement(parser, statement);
        } else if (next_lexeme->isKeyword("STEP")) {
          if (startAsParm2) {
            action = new ActionNode("TO_STEP");
            parser.pushActionNodeRoot(action);
          } else {
            action = new ActionNode(next_lexeme);
            parser.pushActionNodeRoot(action);
          }
          continue;
        } else if (next_lexeme->isSeparator("(")) {
          parmCount++;
          if (startAsParm2)
            state = 3;
          else
            state++;
          if (ctx.actionRoot->lexeme->value != "STEP" &&
              ctx.actionRoot->lexeme->value != "TO_STEP") {
            action = new ActionNode("COORD");
            if (startAsParm2) action->lexeme->name = "TO_COORD";
            action->lexeme->value = action->lexeme->name;
            parser.pushActionNodeRoot(action);
          }
          continue;
        } else if (next_lexeme->isOperator("-")) {
          startAsParm2 = true;
          continue;
        } else {
          ctx.error_message = "LINE without a valid complement.";
          ctx.eval_expr_error = true;
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
              ctx.error_message = "Invalid parentheses syntax in LINE statement.";
              ctx.eval_expr_error = true;
              return false;
            }
            if (parm.getLexemeCount()) {
              parm.setLexemeBOF();
              if (!parser.evalExpressionTokens(&parm)) {
                return false;
              }
              parm.clearLexemes();
              parser.popActionNodeRoot();
            }
            sepTime = true;
            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!parser.evalExpressionTokens(&parm)) {
              return false;
            }
            parm.clearLexemes();
          }
          continue;
        } else if (next_lexeme->isOperator("-") && sepTime) {
          state++;
          continue;
        }

      } break;

      case 2: {
        if (next_lexeme->isKeyword("STEP")) {
          action = new ActionNode("TO_STEP");
          parser.pushActionNodeRoot(action);
          continue;
        } else if (next_lexeme->isSeparator("(")) {
          state++;
          if (ctx.actionRoot) {
            if (ctx.actionRoot->lexeme->value != "TO_STEP") {
              action = new ActionNode("TO_COORD");
              parser.pushActionNodeRoot(action);
            }
          } else {
            ctx.error_message = "Invalid parentheses syntax in LINE statement.";
            ctx.eval_expr_error = true;
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
              ctx.error_message = "Invalid parentheses syntax in LINE statement.";
              ctx.eval_expr_error = true;
              return false;
            }
            mustPopAction = true;
            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          parmCount++;

          if (parm.getLexemeCount()) {
            next_lexeme = parm.getFirstLexeme();

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
              next_lexeme = new Lexeme(Lexeme::type_literal,
                                       Lexeme::subtype_numeric, parmValue);
              parser.pushActionFromLexemeNode(next_lexeme);
            } else {
              parm.setLexemeBOF();
              if (!parser.evalExpressionTokens(&parm)) {
                return false;
              }
              if (mustPopAction) {
                parser.popActionNodeRoot();
                mustPopAction = false;
              }
            }

            parm.clearLexemes();

          } else {
            next_lexeme = ctx.lex_null;
            parser.pushActionFromLexemeNode(next_lexeme);
          }

          continue;

        } else if (mustPopAction) {
          ctx.error_message = "Invalid syntax in LINE statement.";
          ctx.eval_expr_error = true;
          return false;
        }

      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    next_lexeme = parm.getFirstLexeme();

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
          int operatorCode = parser.gfxOperatorFromLexeme(next_lexeme);

          isSpecialParameter = true;

          if (operatorCode >= 0) {
            parmValue = to_string(operatorCode);
          } else {
            ctx.error_message = "Invalid operator parameter in LINE statement.";
            ctx.eval_expr_error = true;
            return false;
          }
        }
      }
    }

    if (isSpecialParameter) {
      next_lexeme =
          new Lexeme(Lexeme::type_literal, Lexeme::subtype_numeric, parmValue);
      parser.pushActionFromLexemeNode(next_lexeme);
    } else {
      parm.setLexemeBOF();
      if (!parser.evalExpressionTokens(&parm)) {
        return false;
      }
    }

    parm.clearLexemes();
  }

  return true;
}

bool GraphicsStatementStrategy::parseCircle(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  LexerLine parm;
  int state = 0, sepCount = 0, parmCount = 0;
  bool mustPopAction = false;
  ParserContext& ctx = parser.getContext();

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = parser.coalesceLexeme(next_lexeme);

    if (state == 0) {
      if (next_lexeme->isKeyword("STEP")) {
        action = new ActionNode(next_lexeme);
        parser.pushActionNodeRoot(action);
        continue;
      } else if (next_lexeme->isSeparator("(")) {
        state++;
        parmCount++;
        if (ctx.actionRoot->lexeme->value != "STEP") {
          action = new ActionNode("COORD");
          parser.pushActionNodeRoot(action);
        }
        continue;
      } else {
        ctx.error_message = "CIRCLE without a valid complement.";
        ctx.eval_expr_error = true;
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
        if (parm.getLexemeCount()) {
          parm.setLexemeBOF();
          if (!parser.evalExpressionTokens(&parm)) {
            return false;
          }
          parm.clearLexemes();
          if (mustPopAction) {
            parser.popActionNodeRoot();
            mustPopAction = false;
          } else
            parmCount++;
        } else {
          parmCount++;
          next_lexeme = ctx.lex_null;
          parser.pushActionFromLexemeNode(next_lexeme);
        }
        continue;
      }
      parm.addLexeme(next_lexeme);
    }
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!parser.evalExpressionTokens(&parm)) {
      return false;
    }

    parm.clearLexemes();
  }

  return true;
}

bool GraphicsStatementStrategy::parsePaint(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  LexerLine parm;
  int state = 0, sepCount = 0, parmCount = 0;
  bool mustPopAction = false;
  ParserContext& ctx = parser.getContext();

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = parser.coalesceLexeme(next_lexeme);

    if (state == 0) {
      if (next_lexeme->isKeyword("STEP")) {
        action = new ActionNode(next_lexeme);
        parser.pushActionNodeRoot(action);
        continue;
      } else if (next_lexeme->isSeparator("(")) {
        state++;
        parmCount++;
        if (ctx.actionRoot->lexeme->value != "STEP") {
          action = new ActionNode("COORD");
          parser.pushActionNodeRoot(action);
        }
        continue;
      } else {
        ctx.error_message = "PAINT without a valid complement.";
        ctx.eval_expr_error = true;
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
        if (parm.getLexemeCount()) {
          parm.setLexemeBOF();
          if (!parser.evalExpressionTokens(&parm)) {
            return false;
          }
          parm.clearLexemes();
          if (mustPopAction) {
            parser.popActionNodeRoot();
            mustPopAction = false;
          } else
            parmCount++;
        } else {
          next_lexeme = ctx.lex_null;
          parser.pushActionFromLexemeNode(next_lexeme);
        }
        continue;
      }
      parm.addLexeme(next_lexeme);
    }
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!parser.evalExpressionTokens(&parm)) {
      return false;
    }

    parm.clearLexemes();
  }

  return true;
}

bool GraphicsStatementStrategy::parseCopy(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  LexerLine parm;
  int state = 0, sepCount = 0, parmCount = 0;
  bool isKeyword = false;
  string parmValue;
  ParserContext& ctx = parser.getContext();

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = parser.coalesceLexeme(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("SCREEN")) {
          parser.pushActionFromLexemeNode(next_lexeme);
          GenericStatementStrategy genericStrategy;
          return genericStrategy.parseStatement(parser, statement);
        }
        if (next_lexeme->isSeparator("(")) {
          parmCount++;
          state = 1;

          action = new ActionNode("COORD");
          parser.pushActionNodeRoot(action);

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
            if (parm.getLexemeCount()) {
              parm.setLexemeBOF();
              if (!parser.evalExpressionTokens(&parm)) {
                return false;
              }
              parm.clearLexemes();
            } else {
              next_lexeme = ctx.lex_null;
              parser.pushActionFromLexemeNode(next_lexeme);
            }
            parser.popActionNodeRoot();
            state = 10;
            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!parser.evalExpressionTokens(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            next_lexeme = ctx.lex_null;
            parser.pushActionFromLexemeNode(next_lexeme);
          }
          continue;
        }

      } break;

      case 2: {
        if (next_lexeme->isKeyword("STEP")) {
          action = new ActionNode("TO_STEP");
          parser.pushActionNodeRoot(action);
          continue;
        } else if (next_lexeme->isSeparator("(")) {
          parmCount++;
          state = 3;
          if (ctx.actionRoot->lexeme->value != "TO_STEP") {
            action = new ActionNode("TO_COORD");
            parser.pushActionNodeRoot(action);
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
            if (parm.getLexemeCount()) {
              parm.setLexemeBOF();
              if (!parser.evalExpressionTokens(&parm)) {
                return false;
              }
              parm.clearLexemes();
            } else {
              next_lexeme = ctx.lex_null;
              parser.pushActionFromLexemeNode(next_lexeme);
            }

            parser.popActionNodeRoot();

            parmCount++;
            state = 4;

            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!parser.evalExpressionTokens(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            next_lexeme = ctx.lex_null;
            parser.pushActionFromLexemeNode(next_lexeme);
          }

          continue;
        }

      } break;

      case 4: {
        if (next_lexeme->isKeyword("TO")) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!parser.evalExpressionTokens(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            next_lexeme = ctx.lex_null;
            parser.pushActionFromLexemeNode(next_lexeme);
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

          action = new ActionNode("TO_DEST");
          parser.pushActionNodeRoot(action);

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
            if (parm.getLexemeCount()) {
              parm.setLexemeBOF();
              if (!parser.evalExpressionTokens(&parm)) {
                return false;
              }
              parm.clearLexemes();
            } else {
              next_lexeme = ctx.lex_null;
              parser.pushActionFromLexemeNode(next_lexeme);
            }

            parser.popActionNodeRoot();

            state = 7;

            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!parser.evalExpressionTokens(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            next_lexeme = ctx.lex_null;
            parser.pushActionFromLexemeNode(next_lexeme);
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
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!parser.evalExpressionTokens(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            next_lexeme = ctx.lex_null;
            parser.pushActionFromLexemeNode(next_lexeme);
          }

          parmCount++;

          continue;
        }

      } break;

      case 9: {
        if (next_lexeme->isKeyword("TO")) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!parser.evalExpressionTokens(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            next_lexeme = ctx.lex_null;
            parser.pushActionFromLexemeNode(next_lexeme);
          }

          if (parmCount == 0) {
            parser.pushActionFromLexemeNode(ctx.lex_null);
          }

          parmCount++;
          state = 5;

          continue;

        } else if (next_lexeme->isSeparator(",")) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!parser.evalExpressionTokens(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            next_lexeme = ctx.lex_null;
            parser.pushActionFromLexemeNode(next_lexeme);
          }

          continue;
        }

      } break;

      case 10: {
        if (next_lexeme->isOperator("-")) {
          state = 2;
          continue;
        } else {
          ctx.error_message = "Invalid syntax on COPY statement";
          return false;
        }
      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    next_lexeme = parm.getFirstLexeme();

    if (parmCount >= 5) {
      if ((next_lexeme->type == Lexeme::type_keyword &&
           next_lexeme->subtype != Lexeme::subtype_function) ||
          next_lexeme->type == Lexeme::type_operator) {
        int operatorCode = parser.gfxOperatorFromLexeme(next_lexeme);

        isKeyword = true;

        if (operatorCode >= 0) {
          parmValue = to_string(operatorCode);
        } else {
          ctx.error_message = "Invalid operator parameter in COPY statement.";
          ctx.eval_expr_error = true;
          return false;
        }
      }
    }

    if (isKeyword) {
      next_lexeme =
          new Lexeme(Lexeme::type_literal, Lexeme::subtype_numeric, parmValue);
      parser.pushActionFromLexemeNode(next_lexeme);
    } else {
      parm.setLexemeBOF();
      if (!parser.evalExpressionTokens(&parm)) {
        return false;
      }
    }

    parm.clearLexemes();
  }

  return true;
}

bool GraphicsStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                        Lexeme* lexeme) {
  if (lexeme->value == "PSET" || lexeme->value == "PRESET")
    return parsePset(parser, statement);
  if (lexeme->value == "LINE") return parseLine(parser, statement);
  if (lexeme->value == "CIRCLE") return parseCircle(parser, statement);
  if (lexeme->value == "PAINT") return parsePaint(parser, statement);
  return parseCopy(parser, statement);
}
