#include "print_statement_strategy.h"

#include "lexer.h"
#include "parser.h"

bool PrintStatementStrategy::parseStatement(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  LexerLine parm;
  ActionNode* action;
  int sepcount = 0, state = 0, i;
  bool print_using = false;
  Lexeme* lex_using[5] = {0, 0, 0, 0, 0};
  ParserContext& ctx = parser.getContext();

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = parser.coalesceLexeme(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isSeparator("#") && sepcount == 0) {
          state = 1;
          parser.pushActionFromLexemeNode(next_lexeme);
          continue;
        } else if (next_lexeme->isKeyword("USING")) {
          print_using = true;
          state = 3;
          continue;
        } else if (next_lexeme->isSeparator("(")) {
          sepcount++;
        } else if (next_lexeme->isSeparator(")") && sepcount > 0) {
          sepcount--;
        } else if (next_lexeme->type == Lexeme::type_separator &&
                   (next_lexeme->value == "," || next_lexeme->value == ";") &&
                   sepcount == 0) {
          if (parm.getLexemeCount()) {
            if (print_using) {
              parm.addLexeme(lex_using[4]);
            }

            parm.setLexemeBOF();
            if (!parser.evalExpressionTokens(&parm)) {
              return false;
            }

            parm.clearLexemes();
          }

          action = new ActionNode(next_lexeme);
          ctx.actionRoot->actions.push_back(action);

          continue;
        }
      } break;

      case 1: {
        state = 2;
        parser.pushActionFromLexemeNode(next_lexeme);
        parser.popActionNodeRoot();
        continue;
      } break;

      case 2: {
        if (next_lexeme->isSeparator(",")) {
          state = 0;
          continue;
        }
      } break;

      case 3: {
        if (next_lexeme->type == Lexeme::type_identifier ||
            next_lexeme->type == Lexeme::type_literal) {
          lex_using[0] = new Lexeme(Lexeme::type_keyword,
                                    Lexeme::subtype_function, "USING$");
          lex_using[1] =
              new Lexeme(Lexeme::type_separator, Lexeme::subtype_string, "(");
          lex_using[2] = next_lexeme;
          lex_using[3] =
              new Lexeme(Lexeme::type_separator, Lexeme::subtype_string, ",");
          lex_using[4] =
              new Lexeme(Lexeme::type_separator, Lexeme::subtype_string, ")");

          for (i = 0; i <= 3; i++) parm.addLexeme(lex_using[i]);

          state = 4;

          continue;
        } else {
          return false;
        }
      } break;

      case 4: {
        if (next_lexeme->type == Lexeme::type_separator &&
            (next_lexeme->value == "," || next_lexeme->value == ";")) {
          state = 0;
          continue;
        } else {
          return false;
        }
      } break;
    }

    if (parm.getLexemeCount() == 0 && print_using) {
      for (i = 0; i <= 3; i++) parm.addLexeme(lex_using[i]);
    }

    parm.addLexeme(next_lexeme);
  }

  if (parm.getLexemeCount()) {
    if (print_using) {
      parm.addLexeme(lex_using[4]);
    }
    parm.setLexemeBOF();
    if (!parser.evalExpressionTokens(&parm)) {
      return false;
    }
  }

  return true;
}

bool PrintStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                     Lexeme* lexeme) {
  if (lexeme->value == "?") {
    lexeme->value = "PRINT";
    lexeme->name = lexeme->value;
  }

  return parseStatement(parser, statement);
}
