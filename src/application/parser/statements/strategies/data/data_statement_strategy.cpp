#include "data_statement_strategy.h"

#include "lexeme.h"
#include "lexer_line_context.h"
#include "logger.h"
#include "tag_node.h"

bool DataStatementStrategy::parseData(shared_ptr<ParserContext> context,
                                      shared_ptr<LexerLineContext> statement,
                                      bool isBinaryData) {
  shared_ptr<Lexeme> next_lexeme, lexeme;
  Lexeme::LexemeSubType subtype;
  string stext = "", sname;
  int i, itext;
  char* s;
  bool lastWasSeparator = true;

  if (isBinaryData) {
    context->getHasIData() = true;
    sname = "_IDATA_";
    subtype = Lexeme::subtype_integer_data;
  } else {
    context->getHasData() = true;
    sname = "_DATA_";
    subtype = Lexeme::subtype_string;
  }

  while ((next_lexeme = statement->getNextLexeme())) {
    next_lexeme = context->coalesceSymbols(next_lexeme);

    if (next_lexeme->type == Lexeme::type_separator &&
        (next_lexeme->value == "," || next_lexeme->value == ";")) {
      if (lastWasSeparator) {
        i = context->getDatas().size() + 1;
        lexeme =
            make_shared<Lexeme>(Lexeme::type_literal, Lexeme::subtype_string,
                                sname + to_string(i), "");
        if (lexeme) {
          lexeme->tag = context->getTag()->name;
          context->pushActionFromLexeme(lexeme);
          context->getDatas().push_back(lexeme);
        }

      } else if (stext.size()) {
        i = context->getDatas().size() + 1;
        next_lexeme = make_shared<Lexeme>(Lexeme::type_literal, subtype,
                                          sname + to_string(i), stext);
        next_lexeme->tag = context->getTag()->name;

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
            context->getLogger()->warning(
                "Error while converting numeric constant " + stext);
            itext = 0;
          }
          next_lexeme->value = to_string(itext);
        }

        context->pushActionFromLexeme(next_lexeme);
        context->getDatas().push_back(next_lexeme);

        stext = "";

      } else {
        context->getLogger()->error("Invalid DATA parameter type");
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
    i = context->getDatas().size() + 1;
    lexeme = make_shared<Lexeme>(Lexeme::type_literal, Lexeme::subtype_string,
                                 sname + to_string(i), "");
    if (lexeme) {
      lexeme->tag = context->getTag()->name;
      context->pushActionFromLexeme(lexeme);
      context->getDatas().push_back(lexeme);
    }
  }

  if (stext.size()) {
    i = context->getDatas().size() + 1;
    next_lexeme = make_shared<Lexeme>(Lexeme::type_literal, subtype,
                                      "_DATA_" + to_string(i), stext);
    next_lexeme->tag = context->getTag()->name;

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
        context->getLogger()->warning("Error while converting numeric constant " +
                                 stext);
        itext = 0;
      }
      next_lexeme->value = to_string(itext);
    }

    context->pushActionFromLexeme(next_lexeme);
    context->getDatas().push_back(next_lexeme);
  }

  return true;
}

bool DataStatementStrategy::execute(shared_ptr<ParserContext> context,
                                    shared_ptr<LexerLineContext> statement,
                                    shared_ptr<Lexeme> lexeme) {
  (void)lexeme;
  return parseData(context, statement, false);
}
