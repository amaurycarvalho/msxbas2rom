#include "on_statement_strategy.h"


bool OnStatementStrategy::parseOn(ParserContext& context, LexerLine* statement) {
  Lexeme* next_lexeme;

  if ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = context.coalesceSymbols(next_lexeme);

    if (next_lexeme->isKeyword("ERROR")) {
      return parseOnError(context, statement);
    } else if (next_lexeme->isKeyword("INTERVAL")) {
      context.has_traps = true;
      return parseOnInterval(context, statement);
    } else if (next_lexeme->isKeyword("KEY")) {
      context.has_traps = true;
      return parseOnKey(context, statement);
    } else if (next_lexeme->isKeyword("SPRITE")) {
      context.has_traps = true;
      return parseOnSprite(context, statement);
    } else if (next_lexeme->isKeyword("STOP")) {
      context.has_traps = true;
      return parseOnStop(context, statement);
    } else if (next_lexeme->isKeyword("STRIG")) {
      context.has_traps = true;
      return parseOnStrig(context, statement);
    } else {
      return parseOnGotoGosub(context, statement);
    }
  }

  return false;
}

bool OnStatementStrategy::parseOnGotoGosub(ParserContext& context, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode *action, *action_index;
  LexerLine parm;
  int state = 0;
  bool next_is_sep = false;

  next_lexeme = statement->getPreviousLexeme();
  if (!next_lexeme) return false;

  action_index = new ActionNode(context.lex_index);

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("GOSUB") || next_lexeme->isKeyword("GOTO")) {
          if (!parm.getLexemeCount()) return false;

          context.pushActionRoot(action_index);

          parm.setLexemeBOF();
          if (!evaluateExpression(context, &parm)) {
            return false;
          }
          parm.clearLexemes();

          context.popActionRoot();

          action = new ActionNode(next_lexeme);
          context.pushActionRoot(action);

          state++;
          continue;
        }
      } break;

      case 1: {
        if (next_lexeme->type == Lexeme::type_literal) {
          if (next_is_sep) return false;

          context.pushActionFromLexeme(next_lexeme);
          next_is_sep = true;
          continue;
        } else if (next_lexeme->isSeparator(",")) {
          if (!next_is_sep) {
            context.pushActionFromLexeme(context.lex_null);
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
    context.popActionRoot();
  }

  return state;
}

bool OnStatementStrategy::parseOnError(ParserContext& context, LexerLine* statement) {
  (void)context;
  (void)statement;
  return false;
}

bool OnStatementStrategy::parseOnInterval(ParserContext& context, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode *action, *action_index;
  LexerLine parm;
  int state = 0;
  bool first = true;

  next_lexeme = statement->getCurrentLexeme();
  if (!next_lexeme) return false;

  action = new ActionNode(next_lexeme);
  context.pushActionRoot(action);

  action_index = new ActionNode(context.lex_index);

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

          context.pushActionRoot(action_index);

          parm.setLexemeBOF();
          if (!evaluateExpression(context, &parm)) {
            return false;
          }
          parm.clearLexemes();

          context.popActionRoot();

          action = new ActionNode(next_lexeme);
          context.pushActionRoot(action);

          state++;
          continue;
        }
      } break;

      case 1: {
        if (next_lexeme->type == Lexeme::type_literal) {
          context.pushActionFromLexeme(next_lexeme);
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
    context.popActionRoot();
  }

  context.popActionRoot();

  return state;
}

bool OnStatementStrategy::parseOnKey(ParserContext& context, LexerLine* statement) {
  Lexeme* next_lexeme;
  ActionNode* action;
  int state = 0;
  bool next_is_sep = false;

  next_lexeme = statement->getCurrentLexeme();
  if (!next_lexeme) return false;

  action = new ActionNode(next_lexeme);
  context.pushActionRoot(action);

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("GOSUB")) {
          action = new ActionNode(next_lexeme);
          context.pushActionRoot(action);

          state++;
          continue;
        } else {
          return false;
        }
      } break;

      case 1: {
        if (next_lexeme->type == Lexeme::type_literal) {
          if (next_is_sep) return false;

          context.pushActionFromLexeme(next_lexeme);
          next_is_sep = true;
          continue;
        } else if (next_lexeme->isSeparator(",")) {
          if (!next_is_sep) {
            context.pushActionFromLexeme(context.lex_null);
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
    context.popActionRoot();
  }
  context.popActionRoot();

  return state;
}

bool OnStatementStrategy::parseOnSprite(ParserContext& context, LexerLine* statement) {
  return parseOnKey(context, statement);
}

bool OnStatementStrategy::parseOnStop(ParserContext& context, LexerLine* statement) {
  return parseOnKey(context, statement);
}

bool OnStatementStrategy::parseOnStrig(ParserContext& context, LexerLine* statement) {
  return parseOnKey(context, statement);
}

bool OnStatementStrategy::parseInterval(ParserContext& context, LexerLine* statement) {
  Lexeme* next_lexeme;

  if ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "ON" || next_lexeme->value == "OFF" ||
          next_lexeme->value == "STOP") {
        context.pushActionFromLexeme(next_lexeme);
        return true;
      }
    }
  }

  return false;
}

bool OnStatementStrategy::parseStop(ParserContext& context, LexerLine* statement) {
  Lexeme* next_lexeme;

  if ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "ON" || next_lexeme->value == "OFF" ||
          next_lexeme->value == "STOP") {
        context.pushActionFromLexeme(next_lexeme);
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

  return true;
}

bool OnStatementStrategy::parseKey(ParserContext& context, LexerLine* statement) {
  Lexeme* next_lexeme;
  LexerLine parm;
  int sepCount = 0;

  while ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "ON" || next_lexeme->value == "OFF" ||
          next_lexeme->value == "STOP") {
        if (parm.getLexemeCount()) {
          parm.setLexemeBOF();
          if (!evaluateExpression(context, &parm)) {
            return false;
          }
          parm.clearLexemes();
        }

        context.pushActionFromLexeme(next_lexeme);
        continue;
      }

    } else if (next_lexeme->isSeparator("(")) {
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
      }

      continue;
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

bool OnStatementStrategy::parseStrig(ParserContext& context, LexerLine* statement) {
  return parseKey(context, statement);
}

bool OnStatementStrategy::execute(ParserContext& context, LexerLine* statement, Lexeme* lexeme) {
  if (!lexeme || lexeme->type != Lexeme::type_keyword) return false;

  if (lexeme->value == "ON") return parseOn(context, statement);
  if (lexeme->value == "INTERVAL") return parseInterval(context, statement);
  if (lexeme->value == "STOP") return parseStop(context, statement);
  if (lexeme->value == "KEY") return parseKey(context, statement);
  if (lexeme->value == "STRIG") return parseStrig(context, statement);

  return false;
}
