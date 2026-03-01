#include "set_statement_strategy.h"

#include "generic_statement_strategy.h"
#include "parser.h"

bool SetStatementStrategy::parseSetAdjust(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  LexerLine parm;
  int state = 0, sepCount = 0;
  bool mustPopAction = false;
  ParserContext& ctx = parser.getContext();

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = parser.coalesceLexeme(next_lexeme);

    if (state == 0) {
      if (next_lexeme->isSeparator("(")) {
        state++;
        continue;
      } else {
        ctx.error_message = "SET ADJUST without a valid complement.";
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
          }
        } else {
          parser.pushActionFromLexemeNode(ctx.lex_null);
        }
        continue;
      }
      parm.addLexeme(next_lexeme);
    }
  }

  if (parm.getLexemeCount()) {
    parm.getFirstLexeme();
    parm.setLexemeBOF();
    if (!parser.evalExpressionTokens(&parm)) {
      return false;
    }
    parm.clearLexemes();
  }

  return true;
}

bool SetStatementStrategy::parseSetTile(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  bool result = false;

  if ((next_lexeme = statement->getNextLexeme())) {
    parser.coalesceLexeme(next_lexeme);

    next_lexeme = statement->getCurrentLexeme();
    action = new ActionNode(next_lexeme);
    parser.pushActionNodeRoot(action);

    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "COLOR" || next_lexeme->value == "PATTERN" ||
          next_lexeme->value == "FLIP" || next_lexeme->value == "ROTATE") {
        result = parseSetTileColpat(parser, statement);
      } else if (next_lexeme->value == "ON" || next_lexeme->value == "OFF") {
        result = true;
      }
    }

    parser.popActionNodeRoot();
  }

  return result;
}

bool SetStatementStrategy::parseSetTileColpat(Parser& parser,
                                              LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* act_coord;
  LexerLine parm;
  int state = 1, sepCount = 0;
  bool hasArrayParm = false;
  ParserContext& ctx = parser.getContext();

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = parser.coalesceLexeme(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isSeparator("(")) {
          state = 2;
          sepCount = 0;
          act_coord = new ActionNode("ARRAY");
          parser.pushActionNodeRoot(act_coord);
          hasArrayParm = true;
          continue;
        } else {
          state = 1;
        }
      }

      case 1: {
        if (next_lexeme->isSeparator(",")) {
          state = 0;
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!parser.evalExpressionTokens(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            parser.pushActionFromLexemeNode(ctx.lex_null);
          }
          if (hasArrayParm) {
            parser.popActionNodeRoot();
            hasArrayParm = false;
          }
          continue;
        }
      } break;

      case 2: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount) {
            sepCount--;
          } else {
            state = 1;
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
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!parser.evalExpressionTokens(&parm)) {
      return false;
    }

    parm.clearLexemes();

    if (hasArrayParm) {
      parser.popActionNodeRoot();
    }
  }

  return true;
}

bool SetStatementStrategy::parseSetSprite(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  bool result = false;

  if ((next_lexeme = statement->getNextLexeme())) {
    parser.coalesceLexeme(next_lexeme);

    next_lexeme = statement->getCurrentLexeme();
    action = new ActionNode(next_lexeme);
    parser.pushActionNodeRoot(action);

    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "COLOR" || next_lexeme->value == "PATTERN" ||
          next_lexeme->value == "FLIP" || next_lexeme->value == "ROTATE") {
        result = parseSetSpriteColpattra(parser, statement);
      }
    }

    parser.popActionNodeRoot();
  }

  return result;
}

bool SetStatementStrategy::parseSetSpriteColpattra(Parser& parser,
                                                   LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* act_coord;
  LexerLine parm;
  int state = 1, sepCount = 0;
  bool hasArrayParm = false;
  ParserContext& ctx = parser.getContext();

  parm.clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = parser.coalesceLexeme(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isSeparator("(")) {
          state = 2;
          sepCount = 0;
          act_coord = new ActionNode("ARRAY");
          parser.pushActionNodeRoot(act_coord);
          hasArrayParm = true;
          continue;
        } else {
          state = 1;
        }
      }

      case 1: {
        if (next_lexeme->isSeparator(",")) {
          state = 0;
          if (parm.getLexemeCount()) {
            parm.setLexemeBOF();
            if (!parser.evalExpressionTokens(&parm)) {
              return false;
            }
            parm.clearLexemes();
          } else {
            parser.pushActionFromLexemeNode(ctx.lex_null);
          }
          if (hasArrayParm) {
            parser.popActionNodeRoot();
            hasArrayParm = false;
          }
          continue;
        }
      } break;

      case 2: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          if (sepCount) {
            sepCount--;
          } else {
            state = 1;
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
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!parser.evalExpressionTokens(&parm)) {
      return false;
    }

    parm.clearLexemes();

    if (hasArrayParm) {
      parser.popActionNodeRoot();
    }
  }

  return true;
}

bool SetStatementStrategy::parseStatement(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  bool result = false;

  if ((next_lexeme = statement->getNextLexeme())) {
    parser.coalesceLexeme(next_lexeme);

    next_lexeme = statement->getCurrentLexeme();
    action = new ActionNode(next_lexeme);
    parser.pushActionNodeRoot(action);

    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "BEEP" || next_lexeme->value == "DATE" ||
          next_lexeme->value == "PAGE" || next_lexeme->value == "PASSWORD" ||
          next_lexeme->value == "PROMPT" || next_lexeme->value == "SCREEN" ||
          next_lexeme->value == "SCROLL" || next_lexeme->value == "TIME" ||
          next_lexeme->value == "TITLE" || next_lexeme->value == "VIDEO" ||
          next_lexeme->value == "FONT") {
        GenericStatementStrategy genericStrategy;
        result = genericStrategy.parseStatement(parser, statement);
      } else if (next_lexeme->value == "ADJUST") {
        result = parseSetAdjust(parser, statement);
      } else if (next_lexeme->value == "TILE") {
        result = parseSetTile(parser, statement);
      } else if (next_lexeme->value == "SPRITE") {
        result = parseSetSprite(parser, statement);
      }
    }

    parser.popActionNodeRoot();
  }

  return result;
}

bool SetStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                   Lexeme* lexeme) {
  (void)lexeme;
  return parseStatement(parser, statement);
}
