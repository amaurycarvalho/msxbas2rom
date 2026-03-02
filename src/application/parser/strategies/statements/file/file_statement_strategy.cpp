#include "file_statement_strategy.h"

#include <cctype>

#include "parser.h"

bool FileStatementStrategy::parseOpen(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  char* s;
  string stext;
  int state = 0;
  ParserContext& ctx = parser.getContext();

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->type == Lexeme::type_identifier ||
            next_lexeme->type == Lexeme::type_literal) {
          string str = next_lexeme->value;
          size_t pos;

          if (str.size()) str.erase(str.size() - 1, 1);
          if (str.size()) str.erase(0, 1);
          if (str.size()) {
            while ((pos = str.find(" ")) != string::npos) {
              str.erase(pos, 1);
            }
            for (auto& c : str) c = (char)toupper(c);
          }

          if (next_lexeme->type == Lexeme::type_literal && str == "GRP:") {
            ctx.actionRoot->lexeme->name = "OPEN_GRP";
            ctx.actionRoot->lexeme->value = ctx.actionRoot->lexeme->name;
            return true;
          }
          parser.pushActionFromLexemeNode(next_lexeme);
          state = 1;
          continue;
        } else {
          ctx.error_message = "File name is missing in OPEN statement";
          return false;
        }
      } break;

      case 1: {
        s = (char*)next_lexeme->value.c_str();

        if (next_lexeme->isKeyword("FOR")) {
          state = 2;
          continue;
        } else if (next_lexeme->value == "AS") {
          state = 4;
          continue;
        } else if (s[0] == 'A' && s[1] == 'S' && s[2] == '#') {
          s += 3;
          stext = *s;
          next_lexeme =
              new Lexeme(Lexeme::type_literal, Lexeme::subtype_numeric, stext);
          parser.pushActionFromLexemeNode(next_lexeme);
          state = 5;
          continue;
        } else {
          ctx.error_message = "FOR/AS is missing in OPEN statement";
          return false;
        }
      } break;

      case 2: {
        s = (char*)next_lexeme->value.c_str();

        if (next_lexeme->value == "INPUT" || next_lexeme->value == "OUT" ||
            next_lexeme->value == "APP") {
          parser.pushActionFromLexemeNode(next_lexeme);
          parser.popActionNodeRoot();
          continue;
        } else if (next_lexeme->type == Lexeme::type_keyword &&
                   (next_lexeme->value == "PUT" ||
                    next_lexeme->value == "END")) {
          continue;
        } else if (next_lexeme->value == "AS") {
          state = 4;
          continue;
        } else if (s[0] == 'A' && s[1] == 'S' && s[2] == '#') {
          s += 3;
          stext = *s;
          next_lexeme =
              new Lexeme(Lexeme::type_literal, Lexeme::subtype_numeric, stext);
          parser.pushActionFromLexemeNode(next_lexeme);
          state = 5;
          continue;
        } else {
          ctx.error_message = "Invalid FOR parameter in OPEN statement";
          return false;
        }
      } break;

      case 3: {
        s = (char*)next_lexeme->value.c_str();

        if (next_lexeme->value == "AS") {
          state = 4;
          continue;
        } else if (s[0] == 'A' && s[1] == 'S' && s[2] == '#') {
          s += 3;
          stext = *s;
          next_lexeme =
              new Lexeme(Lexeme::type_literal, Lexeme::subtype_numeric, stext);
          parser.pushActionFromLexemeNode(next_lexeme);
          state = 5;
          continue;
        } else {
          ctx.error_message = "AS is missing in OPEN statement";
          return false;
        }
      } break;

      case 4: {
        if (next_lexeme->isSeparator("#")) continue;

        if (next_lexeme->type == Lexeme::type_identifier ||
            next_lexeme->type == Lexeme::type_literal) {
          parser.pushActionFromLexemeNode(next_lexeme);
          state = 5;
          continue;
        } else {
          ctx.error_message = "File number is missing in OPEN statement";
          return false;
        }
      } break;

      case 5: {
        if (next_lexeme->isKeyword("LEN")) {
          state = 6;
          continue;
        } else {
          ctx.error_message = "LEN is missing in OPEN statement";
          return false;
        }
      } break;

      case 6: {
        if (next_lexeme->isOperator("=")) continue;

        if (next_lexeme->type == Lexeme::type_identifier ||
            next_lexeme->type == Lexeme::type_literal) {
          parser.pushActionFromLexemeNode(next_lexeme);
          state = 7;
          continue;
        } else {
          ctx.error_message = "Record length is missing in OPEN statement";
          return false;
        }
      } break;
    }
  }

  return true;
}

bool FileStatementStrategy::parseClose(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  int state = 0;
  ParserContext& ctx = parser.getContext();

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isSeparator("#")) {
          state = 1;
        } else {
          ctx.error_message = "# is missing in CLOSE statement";
          return false;
        }

      } break;

      case 1: {
        if (next_lexeme->isLiteralNumeric()) {
          parser.pushActionFromLexemeNode(next_lexeme);
        } else {
          ctx.error_message = "Invalid parameter in CLOSE statement";
          return false;
        }

      } break;

      case 2: {
        if (next_lexeme->isSeparator(",")) {
          state = 0;
        } else {
          ctx.error_message = "Comma is missing in CLOSE statement";
          return false;
        }
      } break;
    }
  }

  return true;
}

bool FileStatementStrategy::parseMaxfiles(Parser& parser, LexerLine* statement) {
  Lexeme* next_lexeme;
  LexerLine parm;
  int state = 0;
  ParserContext& ctx = parser.getContext();

  next_lexeme = statement->getCurrentLexeme();
  if (!next_lexeme) return false;

  next_lexeme->name = "MAXFILES";
  next_lexeme->value = next_lexeme->name;

  while ((next_lexeme = statement->getNextLexeme())) {
    switch (state) {
      case 0: {
        if (next_lexeme->isKeyword("FILES")) {
          state = 1;
          continue;
        } else {
          ctx.error_message = "Invalid MAXFILES assignment";
          return false;
        }
      } break;

      case 1: {
        if (next_lexeme->isOperator("=")) {
          state = 2;
        } else {
          ctx.error_message = "MAXFILES assignment is missing";
          return false;
        }
      } break;

      case 2: {
        parm.addLexeme(next_lexeme);
      }
    }
  }

  if (parm.getLexemeCount()) {
    parm.setLexemeBOF();
    if (!parser.evalExpressionTokens(&parm)) {
      return false;
    }
    parm.clearLexemes();
  }

  return true;
}

bool FileStatementStrategy::execute(Parser& parser, LexerLine* statement,
                                    Lexeme* lexeme) {
  if (lexeme->value == "OPEN") return parseOpen(parser, statement);
  if (lexeme->value == "CLOSE") return parseClose(parser, statement);
  return parseMaxfiles(parser, statement);
}
