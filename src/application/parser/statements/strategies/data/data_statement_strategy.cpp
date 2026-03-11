#include "data_statement_strategy.h"

#include "lexer.h"
#include "logger.h"

bool DataStatementStrategy::parseData(ParserContext& context,
                                      LexerLine* statement, bool isBinaryData) {
  Lexeme *next_lexeme, *lexeme;
  Lexeme::LexemeSubType subtype;
  string stext = "", sname;
  int i, itext;
  char* s;
  bool lastWasSeparator = true;

  if (isBinaryData) {
    context.has_idata = true;
    sname = "_IDATA_";
    subtype = Lexeme::subtype_integer_data;
  } else {
    context.has_data = true;
    sname = "_DATA_";
    subtype = Lexeme::subtype_string;
  }

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = context.coalesceSymbols(next_lexeme);

    if (next_lexeme->type == Lexeme::type_separator &&
        (next_lexeme->value == "," || next_lexeme->value == ";")) {
      if (lastWasSeparator) {
        i = context.datas.size() + 1;
        lexeme = new Lexeme(Lexeme::type_literal, Lexeme::subtype_string,
                            sname + to_string(i), "");
        if (lexeme) {
          lexeme->tag = context.tag->name;
          context.pushActionFromLexeme(lexeme);
          context.datas.push_back(lexeme);
        }

      } else if (stext.size()) {
        i = context.datas.size() + 1;
        next_lexeme = new Lexeme(Lexeme::type_literal, subtype,
                                 sname + to_string(i), stext);
        next_lexeme->tag = context.tag->name;

        s = (char*)stext.c_str();
        if (s[0] == '&') {
          try {
            if (s[1] == 'h' || s[1] == 'H')
              itext = stoi(stext.substr(2), 0, 16);
            else if (s[1] == 'o' || s[1] == 'O')
              itext = stoi(stext.substr(2), 0, 8);
            else if (s[1] == 'b' || s[1] == 'B')
              itext = stoi(stext.substr(2), 0, 2);
            else
              itext = 0;
          } catch (exception& e) {
            printf("Warning: error while converting numeric constant %s\n",
                   stext.c_str());
            itext = 0;
          }
          next_lexeme->value = to_string(itext);
        }

        context.pushActionFromLexeme(next_lexeme);
        context.datas.push_back(next_lexeme);

        stext = "";

      } else {
        context.logger->error("Invalid DATA parameter type");
        return false;
      }

      lastWasSeparator = true;
      continue;

    } else if (next_lexeme->isOperator("'")) {
      break;

    } else {
      stext += next_lexeme->value;
      lastWasSeparator = false;
    }
  }

  if (lastWasSeparator) {
    i = context.datas.size() + 1;
    lexeme = new Lexeme(Lexeme::type_literal, Lexeme::subtype_string,
                        sname + to_string(i), "");
    if (lexeme) {
      lexeme->tag = context.tag->name;
      context.pushActionFromLexeme(lexeme);
      context.datas.push_back(lexeme);
    }
  }

  if (stext.size()) {
    i = context.datas.size() + 1;
    next_lexeme = new Lexeme(Lexeme::type_literal, subtype,
                             "_DATA_" + to_string(i), stext);
    next_lexeme->tag = context.tag->name;

    s = (char*)stext.c_str();
    if (s[0] == '&') {
      try {
        if (s[1] == 'h' || s[1] == 'H')
          itext = stoi(stext.substr(2), 0, 16);
        else if (s[1] == 'o' || s[1] == 'O')
          itext = stoi(stext.substr(2), 0, 8);
        else if (s[1] == 'b' || s[1] == 'B')
          itext = stoi(stext.substr(2), 0, 2);
        else
          itext = 0;
      } catch (exception& e) {
        printf("Warning: error while converting numeric constant %s\n",
               stext.c_str());
        itext = 0;
      }
      next_lexeme->value = to_string(itext);
    }

    context.pushActionFromLexeme(next_lexeme);
    context.datas.push_back(next_lexeme);
  }

  return true;
}

bool DataStatementStrategy::execute(ParserContext& context,
                                    LexerLine* statement, Lexeme* lexeme) {
  (void)lexeme;
  return parseData(context, statement, false);
}
