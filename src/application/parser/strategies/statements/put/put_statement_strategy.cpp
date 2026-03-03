#include "put_statement_strategy.h"

#include "parser.h"

bool PutStatementStrategy::parsePutSprite(ParserContext& context, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode *action, *act_coord;
  LexerLine parm;
  int state = 0, sepCount = 0;

  act_coord = new ActionNode("COORD");

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = context.coalesceSymbols(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isSeparator(",")) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!evaluateExpression(context, &parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            context.pushActionFromLexeme(context.lex_null);
          }
          state = 1;
          continue;
        }
      } break;

      case 1: {
        if (next_lexeme->isKeyword("STEP")) {
          action = new ActionNode(next_lexeme);
          context.pushActionRoot(action);
          continue;
        } else if (next_lexeme->isSeparator("(")) {
          state = 2;
          if (context.actionRoot) {
            if (context.actionRoot->lexeme) {
              if (context.actionRoot->lexeme->value != "STEP") {
                context.pushActionRoot(act_coord);
              }
            }
          }
          continue;
        } else if (next_lexeme->isSeparator(",")) {
          state = 3;
          context.pushActionRoot(act_coord);
          context.pushActionFromLexeme(context.lex_null);
          context.pushActionFromLexeme(context.lex_null);
          context.popActionRoot();
          continue;
        } else {
          context.error_message = "PUT SPRITE without a valid complement.";
          context.eval_expr_error = true;
          return false;
        }
      } break;

      case 2: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount) {
            sepCount--;
          } else {
            state = 3;
            if (parm.getLexemeCount()) {
              parm.setLexemeBOF();
              if (!evaluateExpression(context, &parm)) {
                return false;
              }
              parm.clearLexemes();
            } else {
              if (context.actionRoot) {
                context.pushActionFromLexeme(context.lex_null);
                if (context.actionRoot->actions.size() == 1) {
                  context.pushActionFromLexeme(context.lex_null);
                }
              }
            }
            context.popActionRoot();
            next_lexeme = statement->getNextLexeme();
            if (next_lexeme) {
              if (next_lexeme->type != Lexeme::type_separator ||
                  next_lexeme->value != ",") {
                return false;
              }
            }
            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!evaluateExpression(context, &parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            context.pushActionFromLexeme(context.lex_null);
          }
          continue;
        }

      } break;

      case 3: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount) sepCount--;
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!evaluateExpression(context, &parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            context.pushActionFromLexeme(context.lex_null);
          }
          continue;
        }

      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!evaluateExpression(context, &parm)) {
      return false;
    }

    parm.clearLexemes();
  }

  return true;
}

bool PutStatementStrategy::parsePutTile(ParserContext& context, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode *action, *act_coord;
  LexerLine parm;
  int state = 0, sepCount = 0;

  act_coord = new ActionNode("COORD");

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = context.coalesceSymbols(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isSeparator(",")) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!evaluateExpression(context, &parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            context.pushActionFromLexeme(context.lex_null);
          }
          state = 1;
          continue;
        }
      } break;

      case 1: {
        if (next_lexeme->isKeyword("STEP")) {
          action = new ActionNode(next_lexeme);
          context.pushActionRoot(action);
          continue;
        } else if (next_lexeme->isSeparator("(")) {
          state = 2;
          if (context.actionRoot) {
            if (context.actionRoot->lexeme) {
              if (context.actionRoot->lexeme->value != "STEP") {
                context.pushActionRoot(act_coord);
              }
            }
          }
          continue;
        } else if (next_lexeme->isSeparator(",")) {
          state = 3;
          context.pushActionRoot(act_coord);
          context.pushActionFromLexeme(context.lex_null);
          context.pushActionFromLexeme(context.lex_null);
          context.popActionRoot();
          continue;
        } else {
          context.error_message = "PUT TILE without a valid complement.";
          context.eval_expr_error = true;
          return false;
        }
      } break;

      case 2: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount) {
            sepCount--;
          } else {
            state = 3;
            if (parm.getLexemeCount()) {
              parm.setLexemeBOF();
              if (!evaluateExpression(context, &parm)) {
                return false;
              }
              parm.clearLexemes();
            } else {
              if (context.actionRoot) {
                context.pushActionFromLexeme(context.lex_null);
                if (context.actionRoot->actions.size() == 1) {
                  context.pushActionFromLexeme(context.lex_null);
                }
              }
            }
            context.popActionRoot();
            next_lexeme = statement->getNextLexeme();
            if (next_lexeme) {
              if (next_lexeme->type != Lexeme::type_separator ||
                  next_lexeme->value != ",") {
                return false;
              }
            }
            continue;
          }
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!evaluateExpression(context, &parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            context.pushActionFromLexeme(context.lex_null);
          }
          continue;
        }

      } break;

      case 3: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount) sepCount--;
        } else if (next_lexeme->isSeparator(",") && sepCount == 0) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!evaluateExpression(context, &parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            context.pushActionFromLexeme(context.lex_null);
          }
          continue;
        }

      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!evaluateExpression(context, &parm)) {
      return false;
    }

    parm.clearLexemes();
  }

  return true;
}

bool PutStatementStrategy::parseStatement(ParserContext& context, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  bool result = false;

  if ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = context.coalesceSymbols(next_lexeme);

    if (next_lexeme->type == Lexeme::type_keyword) {
      action = new ActionNode(next_lexeme);
      context.pushActionRoot(action);

      if (next_lexeme->value == "SPRITE") {
        result = parsePutSprite(context, statement);
      } else if (next_lexeme->value == "TILE") {
        result = parsePutTile(context, statement);
      }

      context.popActionRoot();
    }
  }

  return result;
}

bool PutStatementStrategy::execute(ParserContext& context, LexerLine* statement, Lexeme* lexeme) {
  (void)lexeme;
  return parseStatement(context, statement);
}
