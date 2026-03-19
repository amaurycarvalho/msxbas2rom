#include "def_statement_strategy.h"

#include "lexeme.h"
#include "lexer_line_context.h"
#include "logger.h"

bool DefStatementStrategy::parseDefUsr(shared_ptr<ParserContext> context,
                                       shared_ptr<LexerLineContext> statement) {
  shared_ptr<Lexeme> next_lexeme;
  shared_ptr<LexerLineContext> parm = make_shared<LexerLineContext>();
  int state = 0;

  context->has_defusr = true;

  next_lexeme = statement->getCurrentLexeme();
  if (!next_lexeme) {
    return false;
  }

  if (next_lexeme->value == "USR0") {
    next_lexeme->name = "USR";
    next_lexeme->value = next_lexeme->name;
    context->pushActionFromLexeme(context->lex_zero);
    state = 1;
  }

  context->pushActionFromLexeme(next_lexeme);

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isLiteralNumeric()) {
          state = 1;
          context->pushActionFromLexeme(next_lexeme);
          continue;
        } else if (next_lexeme->isOperator("=")) {
          state = 2;
          context->pushActionFromLexeme(context->lex_zero);
        } else {
          context->logger->error("Invalid DEF USR assignment");
          return false;
        }
      } break;

      case 1: {
        if (next_lexeme->isOperator("=")) {
          state = 2;
        } else {
          context->logger->error("DEF USR assignment is missing");
          return false;
        }
      } break;

      case 2: {
        parm->addLexeme(next_lexeme);
      }
    }
  }

  if (parm->getLexemeCount()) {
    parm->setLexemeBOF();
    if (!evaluateExpression(context, parm)) {
      return false;
    }
    parm->clearLexemes();
  }

  context->popActionRoot();

  return true;
}

bool DefStatementStrategy::parseWithType(shared_ptr<ParserContext> context,
                                         shared_ptr<LexerLineContext> statement,
                                         int vartype) {
  shared_ptr<Lexeme> next_lexeme;
  int state = 0, c[2], i;

  if (vartype == 0) {
    if ((next_lexeme = statement->getNextLexeme())) {
      next_lexeme = context->coalesceSymbols(next_lexeme);
      if (next_lexeme->name == "INT")
        return parseWithType(context, statement, 2);
      else if (next_lexeme->name == "STR")
        return parseWithType(context, statement, 3);
      else if (next_lexeme->name == "SNG")
        return parseWithType(context, statement, 4);
      else if (next_lexeme->name == "DBL")
        return parseWithType(context, statement, 8);
      else if (next_lexeme->name == "USR")
        return parseDefUsr(context, statement);
      else if (next_lexeme->name == "USR0")
        return parseDefUsr(context, statement);
      else
        return false;
    } else
      return false;
  }

  c[0] = -1;
  c[1] = -1;

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = context->coalesceSymbols(next_lexeme);

    if (next_lexeme->isSeparator(",")) {
      if (state) {
        if (c[0] >= 0 && c[1] >= 0)
          for (i = c[0]; i <= c[1]; i++) context->deftbl[i] = vartype;
      } else {
        if (c[0] >= 0) context->deftbl[c[0]] = vartype;
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
      for (i = c[0]; i <= c[1]; i++) context->deftbl[i] = vartype;
  } else {
    if (c[0] >= 0) context->deftbl[c[0]] = vartype;
  }

  return true;
}

bool DefStatementStrategy::execute(shared_ptr<ParserContext> context,
                                   shared_ptr<LexerLineContext> statement,
                                   shared_ptr<Lexeme> lexeme) {
  int vartype = 0;

  if (lexeme->value == "DEFINT")
    vartype = 2;
  else if (lexeme->value == "DEFSTR")
    vartype = 3;
  else if (lexeme->value == "DEFSNG")
    vartype = 4;
  else if (lexeme->value == "DEFDBL")
    vartype = 8;

  return parseWithType(context, statement, vartype);
}
