#include "generic_statement_strategy.h"

bool GenericStatementStrategy::parseStatement(ParserContext& context,
                                              LexerLine* statement) {
  Lexeme* next_lexeme;
  LexerLine parm;
  int sepcount = 0;

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = context.coalesceSymbols(next_lexeme);

    if (next_lexeme->isSeparator("(")) {
      sepcount++;
    } else if (next_lexeme->isSeparator(")") && sepcount > 0) {
      sepcount--;
    } else if (next_lexeme->type == Lexeme::type_separator &&
               (next_lexeme->value == "," || next_lexeme->value == ";") &&
               sepcount == 0) {
      if (parm.getLexemeCount()) {
        parm.setLexemeBOF();
        if (!evaluateExpression(context, &parm)) {
          return false;
        }
        parm.clearLexemes();
      } else {
        next_lexeme = context.lex_null.get();
        context.pushActionFromLexeme(next_lexeme);
      }

      continue;
    } else if (next_lexeme->isOperator("'")) {
      break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!evaluateExpression(context, &parm)) {
      return false;
    }
  }

  return true;
}

bool GenericStatementStrategy::execute(ParserContext& context,
                                       LexerLine* statement, Lexeme* lexeme) {
  if (lexeme->value == "BLOAD") context.resourceCount++;
  if (lexeme->value == "PLAY") context.has_play = true;

  return parseStatement(context, statement);
}
