/***
 * @file lexer_line.cpp
 * @brief MSX BASIC lexer line class implementation
 * @author Amaury Carvalho (2019-2025)
 */

#include "lexer_line.h"

#include <ctype.h>
#include <stdio.h>

/***
 * @name LexerLine class functions
 */

LexerLine::LexerLine() {
  lexemeIndex = 0;
}

void LexerLine::clearLexemes() {
  lexemes.clear();
}

void LexerLine::addLexeme(Lexeme* lexeme) {
  lexemes.push_back(lexeme);
}

void LexerLine::setLexemeBOF() {
  lexemeIndex = -1;
}

Lexeme* LexerLine::getCurrentLexeme() {
  return getLexeme(lexemeIndex);
}

Lexeme* LexerLine::getFirstLexeme() {
  return getLexeme(0);
}

Lexeme* LexerLine::getNextLexeme() {
  return getLexeme(lexemeIndex + 1);
}

Lexeme* LexerLine::getPreviousLexeme() {
  return getLexeme(lexemeIndex - 1);
}

Lexeme* LexerLine::getLastLexeme() {
  return getLexeme(lexemes.size() - 1);
}

Lexeme* LexerLine::getLexeme(int i) {
  if (i >= 0 && i < (int)lexemes.size()) {
    lexemeIndex = i;
    return lexemes[i];
  } else
    return 0;
}

int LexerLine::getLexemeCount() {
  return lexemes.size();
}

void LexerLine::pushLexeme() {
  lexemeStack.push(lexemeIndex);
}

void LexerLine::popLexeme() {
  if (!lexemeStack.empty()) {
    lexemeIndex = lexemeStack.top();
    lexemeStack.pop();
  }
}

void LexerLine::popLexemeDiscarding() {
  lexemeStack.pop();
}

void LexerLine::print() {
  printf("%s", line.c_str());
  for (unsigned int i = 0; i < lexemes.size(); i++) lexemes[i]->print();
}

bool LexerLine::evaluate() {
  int i, t;
  char c;
  bool hexa = false;
  Lexeme* lexeme;

  lexemes.clear();

  t = line.length();

  lexeme = new Lexeme();

  for (i = 0; i < t; i++) {
    c = line[i];

    switch (lexeme->type) {
      case Lexeme::type_unknown: {
        if (c <= ' ') {
          continue;
        } else if (isDecimal(c) || c == '"' || c == '&') {
          lexeme->type = Lexeme::type_literal;
          lexeme->value += c;
          hexa = (c == '&');
          if (c == '.')
            lexeme->subtype = Lexeme::subtype_double_decimal;
          else if (c == '"')
            lexeme->subtype = Lexeme::subtype_string;
          else
            lexeme->subtype = Lexeme::subtype_numeric;
          continue;
        } else if (isOperator(c)) {
          lexeme->type = Lexeme::type_operator;
          lexeme->value += c;
          lexemes.push_back(lexeme);
          lexeme = new Lexeme();
          continue;
        } else if (isSeparator(c)) {
          lexeme->type = Lexeme::type_separator;
          lexeme->value += c;
          lexemes.push_back(lexeme);
          lexeme = new Lexeme();
          continue;
        } else if (isIdentifier(c, true)) {
          lexeme->type = Lexeme::type_identifier;
          lexeme->subtype =
              Lexeme::subtype_single_decimal;  // default identifier subtype
          lexeme->value += toupper(c);
          lexeme->name = lexeme->value;
          continue;
        } else if (c == '?' || c == '_') {
          lexeme->type = Lexeme::type_identifier;
          lexeme->subtype = Lexeme::subtype_any;
          lexeme->value += toupper(c);
          lexeme->name = lexeme->value;
          continue;
        } else if (c == '#') {
          lexeme->type = Lexeme::type_separator;
          lexeme->value += c;
          lexemes.push_back(lexeme);
          lexeme = new Lexeme();
          continue;
        } else if (c == '\'') {
          lexeme->type = Lexeme::type_operator;
          lexeme->subtype = Lexeme::subtype_any;
          lexeme->value = "\'";
          lexeme->name = lexeme->value;
          lexemes.push_back(lexeme);
          /// if xbasic special commands...
          if (line[i + 1] == '#') {
            string s = line.substr(i + 1);
            /// Remove trailing character (like newline) if present
            if (!s.empty()) {
              s.pop_back();
            }
            lexeme = new Lexeme(Lexeme::type_comment, Lexeme::subtype_any, s);
            lexemes.push_back(lexeme);
          }
          return true;
        } else {
          lexeme->type = Lexeme::type_unknown;
          lexeme->value += c;
          lexemes.push_back(lexeme);
          return false;
        }
      } break;

      case Lexeme::type_literal: {
        if (lexeme->subtype == Lexeme::subtype_string) {
          lexeme->value += c;
          if (c == '"') {
            lexemes.push_back(lexeme);
            lexeme = new Lexeme();
          }
          continue;
        } else {
          if (hexa) {
            if (isDecimal(c) || isHexDecimal(c)) {
              lexeme->value += c;
            } else {
              lexemes.push_back(lexeme);
              lexeme = new Lexeme();
              if (c > ' ') i--;
            }
          } else if (isDecimal(c)) {
            lexeme->value += c;
            if (lexeme->subtype == Lexeme::subtype_double_decimal &&
                c == '.') {  // strchr(lexeme->value.c_str(), '.' ) ) {
              lexeme->type = Lexeme::type_unknown;
              lexeme->subtype = Lexeme::subtype_any;
              lexemes.push_back(lexeme);
              return false;
            } else {
              if (c == '.') {
                lexeme->subtype = Lexeme::subtype_double_decimal;
              }
              if (lexeme->subtype == Lexeme::subtype_numeric) {
                if (lexeme->value.size() > 5 ||
                    (lexeme->value.size() == 5 && lexeme->value > "32767")) {
                  lexeme->subtype = Lexeme::subtype_single_decimal;
                }
              }
              if (lexeme->subtype == Lexeme::subtype_single_decimal) {
                if (lexeme->value.size() > 6) {
                  lexeme->subtype = Lexeme::subtype_double_decimal;
                }
              }
            }
          } else if (c == '%') {
            lexeme->subtype = Lexeme::subtype_numeric;
          } else if (c == '#') {
            lexeme->subtype = Lexeme::subtype_single_decimal;
          } else if (c == '!') {
            lexeme->subtype = Lexeme::subtype_double_decimal;
          } else {
            lexemes.push_back(lexeme);
            lexeme = new Lexeme();
            if (c > ' ') i--;
          }
          continue;
        }
      } break;

      case Lexeme::type_identifier: {
        if (c <= ' ' || !isIdentifier(c, false)) {
          if (lexeme->isKeyword()) {
            lexeme->type = Lexeme::type_keyword;
            lexeme->subtype = Lexeme::subtype_any;
            if (lexeme->isBooleanOperator()) {
              lexeme->type = Lexeme::type_operator;
              lexeme->subtype = Lexeme::subtype_boolean_operator;
            } else if (lexeme->isFunction()) {
              lexeme->subtype = Lexeme::subtype_function;
            }
          }
          lexemes.push_back(lexeme);
          lexeme = new Lexeme();
          if (c > ' ') i--;
        } else {
          lexeme->value += toupper(c);
          lexeme->name = lexeme->value;
          if (lexeme->isKeyword()) {
            bool isAnotherKeyword = false;
            for (int ii = i + 1; ii < t && ii < (i + 20); ii++) {
              c = line[ii];
              lexeme->value += toupper(c);
              if (lexeme->isKeyword()) {
                isAnotherKeyword = true;
                break;
              }
            }
            lexeme->value = lexeme->name;
            if (isAnotherKeyword) continue;
            lexeme->type = Lexeme::type_keyword;
            lexeme->subtype = Lexeme::subtype_any;
            if (lexeme->isBooleanOperator()) {
              lexeme->type = Lexeme::type_operator;
              lexeme->subtype = Lexeme::subtype_boolean_operator;
            } else if (lexeme->isFunction()) {
              lexeme->subtype = Lexeme::subtype_function;
            }
            lexemes.push_back(lexeme);
            if (lexeme->value == "REM")  //|| lexeme->value == "\'")
              return true;
            lexeme = new Lexeme();
          } else {
            // VALTYP: %=2 $=3 !=4 #=8
            if (c == '%')
              lexeme->subtype = Lexeme::subtype_numeric;
            else if (c == '$')
              lexeme->subtype = Lexeme::subtype_string;
            else if (c == '!')
              lexeme->subtype = Lexeme::subtype_single_decimal;
            else if (c == '#')
              lexeme->subtype = Lexeme::subtype_double_decimal;
          }
        }
        continue;
      } break;

      case Lexeme::type_keyword: {
        if (c <= ' ' || !isIdentifier(c, false)) {
          lexemes.push_back(lexeme);
          lexeme = new Lexeme();
          if (c > ' ') i--;
        } else
          lexeme->value += toupper(c);
        continue;
      } break;

      case Lexeme::type_operator: {
        if (c <= ' ' || !isOperator(c)) {
          lexemes.push_back(lexeme);
          lexeme = new Lexeme();
          if (c > ' ') i--;
        } else
          lexeme->value += c;
        continue;
      } break;

      case Lexeme::type_separator: {
        if (c <= ' ' || !isSeparator(c)) {
          lexemes.push_back(lexeme);
          lexeme = new Lexeme();
          if (c > ' ') i--;
        } else
          lexeme->value += c;
        continue;
      } break;

      case Lexeme::type_comment: {
        if (c <= ' ' || !isComment(c)) {
          lexemes.push_back(lexeme);
          lexeme = new Lexeme();
          if (c > ' ') i--;
        } else
          lexeme->value += c;
        continue;
      } break;
    }
  }

  if (lexeme->type != Lexeme::type_unknown) {
    if (lexeme->isKeyword()) {
      lexeme->type = Lexeme::type_keyword;
      lexeme->subtype = Lexeme::subtype_any;
      if (lexeme->isBooleanOperator()) {
        lexeme->type = Lexeme::type_operator;
        lexeme->subtype = Lexeme::subtype_boolean_operator;
      } else if (lexeme->isFunction()) {
        lexeme->subtype = Lexeme::subtype_function;
      }
    }
    lexemes.push_back(lexeme);
  } else {
    delete lexeme;
  }

  return true;
}

bool LexerLine::isNumeric(char c) {
  return (c >= '0' && c <= '9');
}

bool LexerLine::isDecimal(char c) {
  return isNumeric(c) || c == '.';
}

bool LexerLine::isHexDecimal(char c) {
  return isNumeric(c) || c == 'B' || c == 'b' || c == 'h' || c == 'H' ||
         c == 'o' || c == 'O' || (c >= 'a' && c <= 'f') ||
         (c >= 'A' && c <= 'F');
}

bool LexerLine::isSeparator(char c) {
  return (c == ':' || c == '(' || c == ')' || c == '{' || c == '}' ||
          c == ',' || c == ';');
}

bool LexerLine::isOperator(char c) {
  return (c == '+' || c == '-' || c == '*' || c == '/' || c == '=' ||
          c == '<' || c == '>' || c == '^' ||
          c == '\\');  //|| c == '\''); remark quote symbol
}

bool LexerLine::isIdentifier(char c, bool start) {
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' ||
          ((c == '%' || c == '$' || c == '!' || c == '#' ||
            (c >= '0' && c <= '9')) &&
           !start));
}

bool LexerLine::isComment(char c) {
  return false;
}
