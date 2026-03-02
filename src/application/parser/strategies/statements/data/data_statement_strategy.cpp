#include "data_statement_strategy.h"

#include "lexer.h"
#include "parser.h"

bool DataStatementStrategy::parseData(Parser& parser, LexerLine* statement,
                                      bool isBinaryData) {
  Lexeme *next_lexeme, *lexeme;
  ParserContext& ctx = parser.getContext();
  Lexeme::LexemeSubType subtype;
  string stext = "", sname;
  int i, itext;
  char* s;
  bool lastWasSeparator = true;

  if (isBinaryData) {
    ctx.has_idata = true;
    sname = "_IDATA_";
    subtype = Lexeme::subtype_integer_data;
  } else {
    ctx.has_data = true;
    sname = "_DATA_";
    subtype = Lexeme::subtype_string;
  }

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = parser.coalesceLexeme(next_lexeme);

    if (next_lexeme->type == Lexeme::type_separator &&
        (next_lexeme->value == "," || next_lexeme->value == ";")) {
      if (lastWasSeparator) {
        i = ctx.datas.size() + 1;
        lexeme = new Lexeme(Lexeme::type_literal, Lexeme::subtype_string,
                            sname + to_string(i), "");
        if (lexeme) {
          lexeme->tag = ctx.tag->name;
          parser.pushActionFromLexemeNode(lexeme);
          ctx.datas.push_back(lexeme);
        }

      } else if (stext.size()) {
        i = ctx.datas.size() + 1;
        next_lexeme = new Lexeme(Lexeme::type_literal, subtype,
                                 sname + to_string(i), stext);
        next_lexeme->tag = ctx.tag->name;

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

        parser.pushActionFromLexemeNode(next_lexeme);
        ctx.datas.push_back(next_lexeme);

        stext = "";

      } else {
        ctx.error_message = "Invalid DATA parameter type";
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
    i = ctx.datas.size() + 1;
    lexeme = new Lexeme(Lexeme::type_literal, Lexeme::subtype_string,
                        sname + to_string(i), "");
    if (lexeme) {
      lexeme->tag = ctx.tag->name;
      parser.pushActionFromLexemeNode(lexeme);
      ctx.datas.push_back(lexeme);
    }
  }

  if (stext.size()) {
    i = ctx.datas.size() + 1;
    next_lexeme = new Lexeme(Lexeme::type_literal, subtype,
                             "_DATA_" + to_string(i), stext);
    next_lexeme->tag = ctx.tag->name;

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

    parser.pushActionFromLexemeNode(next_lexeme);
    ctx.datas.push_back(next_lexeme);
  }

  return true;
}

bool DataStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                    Lexeme* lexeme) {
  (void)lexeme;
  return parseData(parser, statement, false);
}
