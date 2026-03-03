#include "color_statement_strategy.h"

#include "generic_statement_strategy.h"

bool ColorStatementStrategy::parseColorRgb(ParserContext& context, LexerLine* statement) {
  Lexeme *next_lexeme, *lex_rgb;
  LexerLine parm;
  int state = 0, sepCount = 0;

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isSeparator("(")) {
          lex_rgb = new Lexeme(Lexeme::type_keyword, Lexeme::subtype_any, "RGB");
          context.pushActionFromLexeme(lex_rgb);

          state++;
          sepCount++;
          continue;

        } else if (next_lexeme->isKeyword("NEW") ||
                   next_lexeme->isKeyword("RESTORE")) {
          context.pushActionFromLexeme(next_lexeme);
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
          parm.setLexemeBOF();
          if (!evaluateExpression(context, &parm)) {
            return false;
          }
          parm.clearLexemes();

          context.popActionRoot();

          return true;

        } else if (next_lexeme->isSeparator(",")) {
          parm.setLexemeBOF();
          if (!evaluateExpression(context, &parm)) {
            return false;
          }
          parm.clearLexemes();

        } else {
          parm.addLexeme(next_lexeme);
        }

      } break;
    }
  }

  return false;
}

bool ColorStatementStrategy::parseColorSprite(ParserContext& context, LexerLine* statement) {
  Lexeme* next_lexeme;
  LexerLine parm;
  int state = 0, sepCount = 0;

  next_lexeme = statement->getCurrentLexeme();
  if (!next_lexeme) return false;

  context.pushActionFromLexeme(next_lexeme);

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
        parm.setLexemeBOF();
        if (!evaluateExpression(context, &parm)) {
          return false;
        }
        parm.clearLexemes();

        state++;
        continue;
      } else {
        parm.addLexeme(next_lexeme);
      }

    } else if (state == 2) {
      if (next_lexeme->isOperator("=")) {
        state++;
        continue;
      } else
        return false;

    } else {
      parm.addLexeme(next_lexeme);
    }
  }

  if (state < 3) return false;

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!evaluateExpression(context, &parm)) {
      return false;
    }
  } else
    return false;

  return true;
}

bool ColorStatementStrategy::parseStatement(ParserContext& context, LexerLine* statement) {
  Lexeme* next_lexeme = statement->getNextLexeme();

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
    context.error_message = "Invalid COLOR statement";
  }

  return false;
}

bool ColorStatementStrategy::execute(ParserContext& context, LexerLine* statement, Lexeme* lexeme) {
  (void)lexeme;
  return parseStatement(context, statement);
}
