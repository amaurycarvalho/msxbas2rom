#include "screen_statement_strategy.h"

#include "action_node.h"
#include "generic_statement_strategy.h"
#include "lexeme.h"
#include "lexer_line_context.h"

bool ScreenStatementStrategy::parseScreenCopy(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<Lexeme> next_lexeme = statement->getCurrentLexeme();
  shared_ptr<LexerLineContext> parm = make_shared<LexerLineContext>();
  int state = 0;
  bool result = false;

  context->pushActionFromLexeme(next_lexeme);
  parm->clearLexemes();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = context->coalesceSymbols(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("TO")) {
          state = 1;
          continue;
        } else {
          return false;
        }
      } break;

      case 1: {
        if (next_lexeme->isKeyword("SCROLL")) {
          if (parm->getLexemeCount()) {
            parm->setLexemeBOF();
            if (!evaluateExpression(context, parm)) {
              return false;
            }
            parm->clearLexemes();
          } else {
            return false;
          }
          state = 2;
          continue;
        }
      } break;
    }

    parm->addLexeme(next_lexeme);
  }

  if (parm->getLexemeCount()) {
    parm->setLexemeBOF();
    if (!evaluateExpression(context, parm)) {
      return false;
    }

    result = true;
  }

  context->popActionRoot();
  return result;
}

bool ScreenStatementStrategy::parseScreenPaste(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<Lexeme> next_lexeme = statement->getCurrentLexeme();
  bool result = false;

  context->pushActionFromLexeme(next_lexeme);

  if ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->isKeyword("FROM")) {
      GenericStatementStrategy genericStrategy;
      result = genericStrategy.parseStatement(context, statement);
    }
  }

  context->popActionRoot();
  return result;
}

bool ScreenStatementStrategy::parseScreenScroll(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<Lexeme> next_lexeme = statement->getCurrentLexeme();
  bool result;

  context->pushActionFromLexeme(next_lexeme);
  GenericStatementStrategy genericStrategy;
  result = genericStrategy.parseStatement(context, statement);
  context->popActionRoot();

  return result;
}

bool ScreenStatementStrategy::parseScreenLoad(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<Lexeme> next_lexeme = statement->getCurrentLexeme();
  bool result;

  context->pushActionFromLexeme(next_lexeme);
  GenericStatementStrategy genericStrategy;
  result = genericStrategy.parseStatement(context, statement);
  context->popActionRoot();

  return result;
}

bool ScreenStatementStrategy::parseScreenOn(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<Lexeme> next_lexeme = statement->getCurrentLexeme();
  context->pushActionFromLexeme(next_lexeme);
  context->popActionRoot();
  return true;
}

bool ScreenStatementStrategy::parseScreenOff(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  return parseScreenOn(context, statement);
}

bool ScreenStatementStrategy::parseStatement(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<Lexeme> next_lexeme;

  if ((next_lexeme = statement->getNextLexeme())) {
    if (next_lexeme->type == Lexeme::type_keyword) {
      if (next_lexeme->value == "COPY") {
        return parseScreenCopy(context, statement);
      } else if (next_lexeme->value == "PASTE") {
        return parseScreenPaste(context, statement);
      } else if (next_lexeme->value == "SCROLL") {
        return parseScreenScroll(context, statement);
      } else if (next_lexeme->value == "LOAD") {
        return parseScreenLoad(context, statement);
      } else if (next_lexeme->value == "ON") {
        return parseScreenOn(context, statement);
      } else if (next_lexeme->value == "OFF") {
        return parseScreenOff(context, statement);
      }
    }

    statement->getPreviousLexeme();
    GenericStatementStrategy genericStrategy;
    return genericStrategy.parseStatement(context, statement);
  }

  return false;
}

bool ScreenStatementStrategy::execute(shared_ptr<ParserContext> context,
                                      shared_ptr<LexerLineContext> statement,
                                      shared_ptr<Lexeme> lexeme) {
  (void)lexeme;
  return parseStatement(context, statement);
}
