#include "on_statement_strategy.h"

#include "parser.h"

bool OnStatementStrategy::parseOn(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;

  if ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = parser.coalesceLexeme(next_lexeme);

    if (next_lexeme->isKeyword("ERROR")) {
      return parseOnError(parser, statement);
    } else if (next_lexeme->isKeyword("INTERVAL")) {
      parser.has_traps = true;
      return parseOnInterval(parser, statement);
    } else if (next_lexeme->isKeyword("KEY")) {
      parser.has_traps = true;
      return parseOnKey(parser, statement);
    } else if (next_lexeme->isKeyword("SPRITE")) {
      parser.has_traps = true;
      return parseOnSprite(parser, statement);
    } else if (next_lexeme->isKeyword("STOP")) {
      parser.has_traps = true;
      return parseOnStop(parser, statement);
    } else if (next_lexeme->isKeyword("STRIG")) {
      parser.has_traps = true;
      return parseOnStrig(parser, statement);
    } else {
      return parseOnGotoGosub(parser, statement);
    }
  }

  return false;
}

bool OnStatementStrategy::parseOnGotoGosub(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode *action, *action_index;
  LexerLine parm;
  int state = 0;
  bool next_is_sep = false;
  ParserContext& ctx = parser.getContext();

  next_lexeme = statement->getPreviousLexeme();
  if (!next_lexeme) return false;

  action_index = new ActionNode(ctx.lex_index);

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("GOSUB") || next_lexeme->isKeyword("GOTO")) {
          if (!parm.getLexemeCount()) return false;

          parser.pushActionNodeRoot(action_index);

          parm.setLexemeBOF();
          if (!parser.evalExpressionTokens(&parm)) {
            return false;
          }
          parm.clearLexemes();

          parser.popActionNodeRoot();

          action = new ActionNode(next_lexeme);
          parser.pushActionNodeRoot(action);

          state++;
          continue;
        }
      } break;

      case 1: {
        if (next_lexeme->type == Lexeme::type_literal) {
          if (next_is_sep) return false;

          parser.pushActionFromLexemeNode(next_lexeme);
          next_is_sep = true;
          continue;
        } else if (next_lexeme->isSeparator(",")) {
          if (!next_is_sep) {
            parser.pushActionFromLexemeNode(ctx.lex_null);
          }

          next_is_sep = false;
          continue;
        } else {
          return false;
        }
      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (state) {
    parser.popActionNodeRoot();
  }

  return state;
}

bool OnStatementStrategy::parseOnError(Parser& parser, LexerLine* statement) {
  (void)parser;
  (void)statement;
  return false;
}

bool OnStatementStrategy::parseOnInterval(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode *action, *action_index;
  LexerLine parm;
  int state = 0;
  bool first = true;
  ParserContext& ctx = parser.getContext();

  next_lexeme = statement->getCurrentLexeme();
  if (!next_lexeme) return false;

  action = new ActionNode(next_lexeme);
  parser.pushActionNodeRoot(action);

  action_index = new ActionNode(ctx.lex_index);

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (first) {
          first = false;
          if (next_lexeme->isOperator("=")) {
            continue;
          }
        }

        if (next_lexeme->isKeyword("GOSUB")) {
          if (!parm.getLexemeCount()) return false;

          parser.pushActionNodeRoot(action_index);

          parm.setLexemeBOF();
          if (!parser.evalExpressionTokens(&parm)) {
            return false;
          }
          parm.clearLexemes();

          parser.popActionNodeRoot();

          action = new ActionNode(next_lexeme);
          parser.pushActionNodeRoot(action);

          state++;
          continue;
        }
      } break;

      case 1: {
        if (next_lexeme->type == Lexeme::type_literal) {
          parser.pushActionFromLexemeNode(next_lexeme);
          state++;
          continue;
        } else {
          return false;
        }
      } break;

      case 2: {
        return false;
      } break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (state) {
    parser.popActionNodeRoot();
  }

  parser.popActionNodeRoot();

  return state;
}

bool OnStatementStrategy::parseOnKey(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  int state = 0;
  bool next_is_sep = false;
  ParserContext& ctx = parser.getContext();

  next_lexeme = statement->getCurrentLexeme();
  if (!next_lexeme) return false;

  action = new ActionNode(next_lexeme);
  parser.pushActionNodeRoot(action);

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("GOSUB")) {
          action = new ActionNode(next_lexeme);
          parser.pushActionNodeRoot(action);

          state++;
          continue;
        } else {
          return false;
        }
      } break;

      case 1: {
        if (next_lexeme->type == Lexeme::type_literal) {
          if (next_is_sep) return false;

          parser.pushActionFromLexemeNode(next_lexeme);
          next_is_sep = true;
          continue;
        } else if (next_lexeme->isSeparator(",")) {
          if (!next_is_sep) {
            parser.pushActionFromLexemeNode(ctx.lex_null);
          }

          next_is_sep = false;
          continue;
        } else {
          return false;
        }
      } break;
    }
  }

  if (state) {
    parser.popActionNodeRoot();
  }
  parser.popActionNodeRoot();

  return state;
}

bool OnStatementStrategy::parseOnSprite(Parser& parser, LexerLine* statement) {
  return parseOnKey(parser, statement);
}

bool OnStatementStrategy::parseOnStop(Parser& parser, LexerLine* statement) {
  return parseOnKey(parser, statement);
}

bool OnStatementStrategy::parseOnStrig(Parser& parser, LexerLine* statement) {
  return parseOnKey(parser, statement);
}

bool OnStatementStrategy::parseInterval(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;

  if ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "ON" || next_lexeme->value == "OFF" ||
          next_lexeme->value == "STOP") {
        parser.pushActionFromLexemeNode(next_lexeme);
        return true;
      }
    }
  }

  return false;
}

bool OnStatementStrategy::parseStop(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;

  if ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "ON" || next_lexeme->value == "OFF" ||
          next_lexeme->value == "STOP") {
        parser.pushActionFromLexemeNode(next_lexeme);
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

  return true;
}

bool OnStatementStrategy::parseKey(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  LexerLine parm;
  int sepCount = 0;

  while ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "ON" || next_lexeme->value == "OFF" ||
          next_lexeme->value == "STOP") {
        if (parm.getLexemeCount()) {
          parm.setLexemeBOF();
          if (!parser.evalExpressionTokens(&parm)) {
            return false;
          }
          parm.clearLexemes();
        }

        parser.pushActionFromLexemeNode(next_lexeme);
        continue;
      }

    } else if (next_lexeme->isSeparator("(")) {
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
      }

      continue;
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

bool OnStatementStrategy::parseStrig(Parser& parser, LexerLine* statement) {
  return parseKey(parser, statement);
}

bool OnStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                  Lexeme* lexeme) {
  if (!lexeme || lexeme->type != Lexeme::type_keyword) return false;

  if (lexeme->value == "ON") return parseOn(parser, statement);
  if (lexeme->value == "INTERVAL") return parseInterval(parser, statement);
  if (lexeme->value == "STOP") return parseStop(parser, statement);
  if (lexeme->value == "KEY") return parseKey(parser, statement);
  if (lexeme->value == "STRIG") return parseStrig(parser, statement);

  return false;
}
