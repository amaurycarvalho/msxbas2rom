#include "generic_statement_strategy.h"

#include "parser.h"

bool GenericStatementStrategy::parseStatement(Parser& parser,
                                              LexerLine* statement) {
  Lexeme* next_lexeme;
  LexerLine parm;
  int sepcount = 0;

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = parser.coalesceLexeme(next_lexeme);

    if (next_lexeme->isSeparator("(")) {
      sepcount++;
    } else if (next_lexeme->isSeparator(")") && sepcount > 0) {
      sepcount--;
    } else if (next_lexeme->type == Lexeme::type_separator &&
               (next_lexeme->value == "," || next_lexeme->value == ";") &&
               sepcount == 0) {
      if (parm.getLexemeCount()) {
        parm.setLexemeBOF();
        if (!parser.evalExpressionTokens(&parm)) {
          return false;
        }
        parm.clearLexemes();
      } else {
        next_lexeme = parser.getContext().lex_null;
        parser.pushActionFromLexemeNode(next_lexeme);
      }

      continue;
    } else if (next_lexeme->isOperator("'")) {
      break;
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!parser.evalExpressionTokens(&parm)) {
      return false;
    }
  }

  return true;
}

bool GenericStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                       Lexeme* lexeme) {
  ParserContext& ctx = parser.getContext();

  if (lexeme->value == "BLOAD") ctx.resourceCount++;
  if (lexeme->value == "PLAY") ctx.has_play = true;

  return parseStatement(parser, statement);
}
