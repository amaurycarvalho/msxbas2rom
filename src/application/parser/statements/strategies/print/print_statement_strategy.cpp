#include "print_statement_strategy.h"

#include "lexer.h"

bool PrintStatementStrategy::parseStatement(ParserContext& context,
                                            LexerLineContext* statement) {
  shared_ptr<Lexeme> next_lexeme;
  LexerLineContext parm;
  ActionNode* action;
  int sepcount = 0, state = 0, i;
  bool print_using = false;
  shared_ptr<Lexeme> lex_using[5] = {0, 0, 0, 0, 0};

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = context.coalesceSymbols(next_lexeme);

    switch (state) {
      case 0: {
        if (next_lexeme->isSeparator("#") && sepcount == 0) {
          state = 1;
          context.pushActionFromLexeme(next_lexeme);
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
            if (!evaluateExpression(context, &parm)) {
              return false;
            }

            parm.clearLexemes();
          }

          action = new ActionNode(next_lexeme);
          context.actionRoot->actions.push_back(action);

          continue;
        }
      } break;

      case 1: {
        state = 2;
        context.pushActionFromLexeme(next_lexeme);
        context.popActionRoot();
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
          lex_using[0] = make_shared<Lexeme>(
              Lexeme::type_keyword, Lexeme::subtype_function, "USING$");
          lex_using[1] = make_shared<Lexeme>(Lexeme::type_separator,
                                             Lexeme::subtype_string, "(");
          lex_using[2] = next_lexeme;
          lex_using[3] = make_shared<Lexeme>(Lexeme::type_separator,
                                             Lexeme::subtype_string, ",");
          lex_using[4] = make_shared<Lexeme>(Lexeme::type_separator,
                                             Lexeme::subtype_string, ")");

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
    if (!evaluateExpression(context, &parm)) {
      return false;
    }
  }

  return true;
}

bool PrintStatementStrategy::execute(ParserContext& context,
                                     LexerLineContext* statement,
                                     shared_ptr<Lexeme> lexeme) {
  if (lexeme->value == "?") {
    lexeme->value = "PRINT";
    lexeme->name = lexeme->value;
  }

  return parseStatement(context, statement);
}
