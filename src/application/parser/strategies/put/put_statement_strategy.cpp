#include "put_statement_strategy.h"

#include "parser.h"

bool PutStatementStrategy::parsePutSprite(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode *action, *act_coord;
  LexerLine parm;
  int state = 0, sepCount = 0;
  ParserContext& ctx = parser.getContext();

  act_coord = new ActionNode("COORD");

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = parser.coalesceLexeme(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isSeparator(",")) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!parser.evalExpressionTokens(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            parser.pushActionFromLexemeNode(ctx.lex_null);
          }
          state = 1;
          continue;
        }
      } break;

      case 1: {
        if (next_lexeme->isKeyword("STEP")) {
          action = new ActionNode(next_lexeme);
          parser.pushActionNodeRoot(action);
          continue;
        } else if (next_lexeme->isSeparator("(")) {
          state = 2;
          if (ctx.actionRoot) {
            if (ctx.actionRoot->lexeme) {
              if (ctx.actionRoot->lexeme->value != "STEP") {
                parser.pushActionNodeRoot(act_coord);
              }
            }
          }
          continue;
        } else if (next_lexeme->isSeparator(",")) {
          state = 3;
          parser.pushActionNodeRoot(act_coord);
          parser.pushActionFromLexemeNode(ctx.lex_null);
          parser.pushActionFromLexemeNode(ctx.lex_null);
          parser.popActionNodeRoot();
          continue;
        } else {
          ctx.error_message = "PUT SPRITE without a valid complement.";
          ctx.eval_expr_error = true;
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
              if (!parser.evalExpressionTokens(&parm)) {
                return false;
              }
              parm.clearLexemes();
            } else {
              if (ctx.actionRoot) {
                parser.pushActionFromLexemeNode(ctx.lex_null);
                if (ctx.actionRoot->actions.size() == 1) {
                  parser.pushActionFromLexemeNode(ctx.lex_null);
                }
              }
            }
            parser.popActionNodeRoot();
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
            if (!parser.evalExpressionTokens(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            parser.pushActionFromLexemeNode(ctx.lex_null);
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
            if (!parser.evalExpressionTokens(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            parser.pushActionFromLexemeNode(ctx.lex_null);
          }
          continue;
        }

      } break;
    }

    parm.addLexeme(next_lexeme);
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

bool PutStatementStrategy::parsePutTile(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode *action, *act_coord;
  LexerLine parm;
  int state = 0, sepCount = 0;
  ParserContext& ctx = parser.getContext();

  act_coord = new ActionNode("COORD");

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = parser.coalesceLexeme(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isSeparator(",")) {
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!parser.evalExpressionTokens(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            parser.pushActionFromLexemeNode(ctx.lex_null);
          }
          state = 1;
          continue;
        }
      } break;

      case 1: {
        if (next_lexeme->isKeyword("STEP")) {
          action = new ActionNode(next_lexeme);
          parser.pushActionNodeRoot(action);
          continue;
        } else if (next_lexeme->isSeparator("(")) {
          state = 2;
          if (ctx.actionRoot) {
            if (ctx.actionRoot->lexeme) {
              if (ctx.actionRoot->lexeme->value != "STEP") {
                parser.pushActionNodeRoot(act_coord);
              }
            }
          }
          continue;
        } else if (next_lexeme->isSeparator(",")) {
          state = 3;
          parser.pushActionNodeRoot(act_coord);
          parser.pushActionFromLexemeNode(ctx.lex_null);
          parser.pushActionFromLexemeNode(ctx.lex_null);
          parser.popActionNodeRoot();
          continue;
        } else {
          ctx.error_message = "PUT TILE without a valid complement.";
          ctx.eval_expr_error = true;
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
              if (!parser.evalExpressionTokens(&parm)) {
                return false;
              }
              parm.clearLexemes();
            } else {
              if (ctx.actionRoot) {
                parser.pushActionFromLexemeNode(ctx.lex_null);
                if (ctx.actionRoot->actions.size() == 1) {
                  parser.pushActionFromLexemeNode(ctx.lex_null);
                }
              }
            }
            parser.popActionNodeRoot();
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
            if (!parser.evalExpressionTokens(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            parser.pushActionFromLexemeNode(ctx.lex_null);
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
            if (!parser.evalExpressionTokens(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            parser.pushActionFromLexemeNode(ctx.lex_null);
          }
          continue;
        }

      } break;
    }

    parm.addLexeme(next_lexeme);
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

bool PutStatementStrategy::parseStatement(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  bool result = false;

  if ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = parser.coalesceLexeme(next_lexeme);

    if (next_lexeme->type == Lexeme::type_keyword) {
      action = new ActionNode(next_lexeme);
      parser.pushActionNodeRoot(action);

      if (next_lexeme->value == "SPRITE") {
        result = parsePutSprite(parser, statement);
      } else if (next_lexeme->value == "TILE") {
        result = parsePutTile(parser, statement);
      }

      parser.popActionNodeRoot();
    }
  }

  return result;
}

bool PutStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                   Lexeme* lexeme) {
  (void)lexeme;
  return parseStatement(parser, statement);
}
