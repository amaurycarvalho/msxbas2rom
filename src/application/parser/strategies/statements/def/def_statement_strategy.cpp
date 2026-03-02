#include "def_statement_strategy.h"

#include "parser.h"

bool DefStatementStrategy::parseDefUsr(Parser& parser, LexerLine* statement) {
  Lexeme *next_lexeme, *lex_zero;
  LexerLine parm;
  int state = 0;
  bool lex_zero_used = false;
  ParserContext& ctx = parser.getContext();

  ctx.has_defusr = true;

  next_lexeme = statement->getCurrentLexeme();
  if (!next_lexeme) {
    return false;
  }

  lex_zero = new Lexeme(Lexeme::type_literal, Lexeme::subtype_numeric, "0");

  if (next_lexeme->value == "USR0") {
    next_lexeme->name = "USR";
    next_lexeme->value = next_lexeme->name;
    parser.pushActionFromLexemeNode(lex_zero);
    lex_zero_used = true;
    state = 1;
  }

  parser.pushActionFromLexemeNode(next_lexeme);

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isLiteralNumeric()) {
          state = 1;
          parser.pushActionFromLexemeNode(next_lexeme);
          continue;
        } else if (next_lexeme->isOperator("=")) {
          state = 2;
          parser.pushActionFromLexemeNode(lex_zero);
          lex_zero_used = true;
        } else {
          ctx.error_message = "Invalid DEF USR assignment";
          if (!lex_zero_used) delete lex_zero;
          return false;
        }
      } break;

      case 1: {
        if (next_lexeme->isOperator("=")) {
          state = 2;
        } else {
          ctx.error_message = "DEF USR assignment is missing";
          if (!lex_zero_used) delete lex_zero;
          return false;
        }
      } break;

      case 2: {
        parm.addLexeme(next_lexeme);
      }
    }
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!parser.evalExpressionTokens(&parm)) {
      if (!lex_zero_used) delete lex_zero;
      return false;
    }
    parm.clearLexemes();
  }

  parser.popActionNodeRoot();

  if (!lex_zero_used) delete lex_zero;

  return true;
}

bool DefStatementStrategy::parseWithType(Parser& parser, LexerLine* statement,
                                         int vartype) {
  Lexeme* next_lexeme;
  int state = 0, c[2], i;
  ParserContext& ctx = parser.getContext();

  if (vartype == 0) {
    if ((next_lexeme = statement->getNextLexeme())) {
      next_lexeme = parser.coalesceLexeme(next_lexeme);
      if (next_lexeme->name == "INT")
        return parseWithType(parser, statement, 2);
      else if (next_lexeme->name == "STR")
        return parseWithType(parser, statement, 3);
      else if (next_lexeme->name == "SNG")
        return parseWithType(parser, statement, 4);
      else if (next_lexeme->name == "DBL")
        return parseWithType(parser, statement, 8);
      else if (next_lexeme->name == "USR")
        return parseDefUsr(parser, statement);
      else if (next_lexeme->name == "USR0")
        return parseDefUsr(parser, statement);
      else
        return false;
    } else
      return false;
  }

  c[0] = -1;
  c[1] = -1;

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = parser.coalesceLexeme(next_lexeme);

    if (next_lexeme->isSeparator(",")) {
      if (state) {
        if (c[0] >= 0 && c[1] >= 0)
          for (i = c[0]; i <= c[1]; i++) ctx.deftbl[i] = vartype;
      } else {
        if (c[0] >= 0) ctx.deftbl[c[0]] = vartype;
      }

      state = 0;
      c[0] = -1;
      c[1] = -1;

    } else if (next_lexeme->isOperator("-")) {
      state = 1;

    } else if (next_lexeme->type == Lexeme::type_identifier) {
      c[state] = next_lexeme->name.c_str()[0] - 'A';
      if (!(c[state] >= 0 && c[state] < 26)) c[state] = -1;
    }
  }

  if (state) {
    if (c[0] >= 0 && c[1] >= 0)
      for (i = c[0]; i <= c[1]; i++) ctx.deftbl[i] = vartype;
  } else {
    if (c[0] >= 0) ctx.deftbl[c[0]] = vartype;
  }

  return true;
}

bool DefStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                   Lexeme* lexeme) {
  int vartype = 0;

  if (lexeme->value == "DEFINT")
    vartype = 2;
  else if (lexeme->value == "DEFSTR")
    vartype = 3;
  else if (lexeme->value == "DEFSNG")
    vartype = 4;
  else if (lexeme->value == "DEFDBL")
    vartype = 8;

  return parseWithType(parser, statement, vartype);
}
