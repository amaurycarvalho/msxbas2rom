/***
 * @file lexeme.h
 * @brief MSX BASIC lexeme class header
 * @author Amaury Carvalho (2019-2025)
 */

#ifndef LEXEME_H_INCLUDED
#define LEXEME_H_INCLUDED

#include <string>

using namespace std;

/***
 * @class Lexeme
 * @brief It represents a simple MSX BASIC lexeme
 *        (constant, keyword, identifier etc)
 */
class Lexeme {
 public:
  enum LexemeType {
    type_unknown = 0,
    type_identifier,  // x, y, up
    type_keyword,     // if, print, color
    type_separator,   // { } ( ) , : ;
    type_operator,    // + - * / < > =
    type_literal,     // true, 6, 1.0, "text"
    type_comment      // text
  } type;

  enum LexemeSubType {
    subtype_any = 0,
    subtype_string,
    subtype_numeric,
    subtype_single_decimal,
    subtype_double_decimal,
    subtype_basic_string,
    subtype_boolean_operator,
    subtype_function,
    subtype_null,
    subtype_binary_data,
    subtype_integer_data,
    subtype_unknown,
  } subtype;

  string name, value;
  int indent;
  string tag;
  bool isAbstract;
  bool isUnary;
  int parm_count;

  bool isArray;
  // @brief number of elements in a row (x)
  int x_size;
  // @brief number of rows (y)
  int y_size;
  // @brief size of an element in bytes
  int x_factor;
  // @brief size of the row in bytes (x_factor * x_size)
  int y_factor;
  // @brief size of the array in bytes
  int array_size;

  void clear();
  Lexeme* clone();
  string toString(int indentOverride = -1);
  char* getTypeName();
  char* getSubTypeName();
  bool isKeyword();
  bool isKeyword(string pvalue);
  bool isSeparator(string pvalue);
  bool isOperator(string pvalue);
  bool isLiteralNumeric();
  bool isBooleanOperator();
  bool isFunction();

  static Lexeme* factory(LexemeType ptype, LexemeSubType psubtype,
                         string pname);
  static Lexeme* factory(LexemeType ptype, LexemeSubType psubtype, string pname,
                         string pvalue);
  static Lexeme* factory(Lexeme* plexeme);

  Lexeme();
  Lexeme(Lexeme* plexeme);
  Lexeme(LexemeType ptype, LexemeSubType psubtype, string pname);
  Lexeme(LexemeType ptype, LexemeSubType psubtype, string pname, string pvalue);
};

#endif  // LEXEME_H_INCLUDED
