#include "color_statement_strategy.h"

#include "generic_statement_strategy.h"
#include "lexeme.h"
#include "lexer_line_context.h"
#include "logger.h"

bool ColorStatementStrategy::parseColorRgb(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<Lexeme> next_lexeme;
  shared_ptr<LexerLineContext> parm = make_shared<LexerLineContext>();
  int state = 0, sepCount = 0;

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isSeparator("(")) {
          context->pushActionFromLexeme(context->lex_rgb);

          state++;
          sepCount++;
          continue;

        } else if (next_lexeme->isKeyword("NEW") ||
                   next_lexeme->isKeyword("RESTORE")) {
          context->pushActionFromLexeme(next_lexeme);
          return true;

        } else
          return false;

      } break;

      case 1: {
        if (next_lexeme->isSeparator("(")) {
          sepCount++;
        } else if (next_lexeme->isSeparator(")")) {
          sepCount--;
        }

        if (next_lexeme->isSeparator(")") && sepCount == 0) {
          parm->setLexemeBOF();
          if (!evaluateExpression(context, parm)) {
            return false;
          }
          parm->clearLexemes();

          context->popActionRoot();

          return true;

        } else if (next_lexeme->isSeparator(",")) {
          parm->setLexemeBOF();
          if (!evaluateExpression(context, parm)) {
            return false;
          }
          parm->clearLexemes();

        } else {
          parm->addLexeme(next_lexeme);
        }

      } break;
    }
  }

  return false;
}

bool ColorStatementStrategy::parseColorSprite(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<Lexeme> next_lexeme;
  shared_ptr<LexerLineContext> parm = make_shared<LexerLineContext>();
  int state = 0, sepCount = 0;

  next_lexeme = statement->getCurrentLexeme();
  if (!next_lexeme) return false;

  context->pushActionFromLexeme(next_lexeme);

  while ((next_lexeme = statement->getNextLexeme())) {
    if (state == 0) {
      if (next_lexeme->isSeparator("(")) {
        state++;
        sepCount++;
        continue;
      } else
        return false;

    } else if (state == 1) {
      if (next_lexeme->isSeparator("(")) {
        sepCount++;
      } else if (next_lexeme->isSeparator(")")) {
        sepCount--;
      }

      if (next_lexeme->isSeparator(")") && sepCount == 0) {
        parm->setLexemeBOF();
        if (!evaluateExpression(context, parm)) {
          return false;
        }
        parm->clearLexemes();

        state++;
        continue;
      } else {
        parm->addLexeme(next_lexeme);
      }

    } else if (state == 2) {
      if (next_lexeme->isOperator("=")) {
        state++;
        continue;
      } else
        return false;

    } else {
      parm->addLexeme(next_lexeme);
    }
  }

  if (state < 3) return false;

  if (parm->getLexemeCount()) {
    parm->setLexemeBOF();
    if (!evaluateExpression(context, parm)) {
      return false;
    }
  } else
    return false;

  return true;
}

bool ColorStatementStrategy::parseStatement(
    shared_ptr<ParserContext> context, shared_ptr<LexerLineContext> statement) {
  shared_ptr<Lexeme> next_lexeme = statement->getNextLexeme();

  if (next_lexeme) {
    if (next_lexeme->isOperator("=")) {
      return parseColorRgb(context, statement);
    } else if (next_lexeme->isKeyword("SPRITE")) {
      return parseColorSprite(context, statement);
    } else if (next_lexeme->isKeyword("SPRITE$")) {
      return parseColorSprite(context, statement);
    } else {
      statement->getPreviousLexeme();
      GenericStatementStrategy genericStrategy;
      return genericStrategy.parseStatement(context, statement);
    }
  } else {
    context->logger->error("Invalid COLOR statement");
  }

  return false;
}

bool ColorStatementStrategy::execute(shared_ptr<ParserContext> context,
                                     shared_ptr<LexerLineContext> statement,
                                     shared_ptr<Lexeme> lexeme) {
  (void)lexeme;
  return parseStatement(context, statement);
}
