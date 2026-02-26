/***
 * @file lexer.h
 * @brief MSX BASIC lexer class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 *   [Lexical analysis](https://en.wikipedia.org/wiki/Lexical_analysis)
 */

#ifndef LEX_H_INCLUDED
#define LEX_H_INCLUDED

#include <array>
#include <stack>
#include <string>
#include <vector>

#include "fswrapper.h"
#include "options.h"

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
  bool isArray;
  bool isAbstract;
  int x_size, y_size;
  int x_factor, y_factor, array_size;
  int parm_count;
  bool isUnary;

  /***
   * @brief Clear the current lexeme
   */
  void clear();

  /***
   * @brief Clone the current lexeme
   * @return A new lexeme copied from current one
   */
  Lexeme* clone();

  /***
   * @brief Print to the terminal the current lexeme
   */
  void print();

  /***
   * @brief Get the type name of current lexeme
   * @return Type name string
   */
  char* getTypeName();

  /***
   * @brief Get the subtype name of current lexeme
   * @return Subtype name string
   */
  char* getSubTypeName();

  /***
   * @brief Is the current lexeme a valid keyword?
   * @return True or false
   * @note [MSX-BASIC
   * Instructions](https://www.msx.org/wiki/Category:MSX-BASIC_Instructions)
   */
  bool isKeyword();

  /***
   * @brief Is the current lexeme a keyword?
   * @param pvalue Keyword to compare against
   * @return True or false
   */
  bool isKeyword(string pvalue);

  /***
   * @brief Is the current lexeme a separator?
   * @param pvalue Separator to compare against
   * @return True or false
   */
  bool isSeparator(string pvalue);

  /***
   * @brief Is the current lexeme an operator?
   * @param pvalue Operator to compare against
   * @return True or false
   */
  bool isOperator(string pvalue);

  /***
   * @brief Is the current lexeme a literal numeric?
   * @return True or false
   */
  bool isLiteralNumeric();

  /***
   * @brief Is the current lexeme a valid boolean operator?
   * @return True or false
   * @note [MSX-BASIC
   * Instructions](https://www.msx.org/wiki/Category:MSX-BASIC_Instructions)
   */
  bool isBooleanOperator();

  /***
   * @brief Is the current lexeme a valid function keyword?
   * @return True or false
   * @note [MSX-BASIC
   * Instructions](https://www.msx.org/wiki/Category:MSX-BASIC_Instructions)
   */
  bool isFunction();

  static Lexeme* factory(LexemeType ptype, LexemeSubType psubtype,
                         string pname);
  static Lexeme* factory(LexemeType ptype, LexemeSubType psubtype, string pname,
                         string pvalue);
  static Lexeme* factory(Lexeme* plexeme);

  /***
   * @brief Lexeme class constructor.
   * It represents a simple MSX BASIC lexeme (constant, keyword, identifier etc)
   */
  Lexeme();

  /***
   * @brief Lexeme class constructor.
   * It represents a simple MSX BASIC lexeme (constant, keyword, identifier etc)
   * @param plexeme Lexeme to clone
   */
  Lexeme(Lexeme* plexeme);

  /***
   * @brief Lexeme class constructor.
   * It represents a simple MSX BASIC lexeme (constant, keyword, identifier etc)
   * @param ptype Lexeme type
   * @param psubtype Lexeme subtype
   * @param pname Lexeme name
   */
  Lexeme(LexemeType ptype, LexemeSubType psubtype, string pname);

  /***
   * @brief Lexeme class constructor.
   * It represents a simple MSX BASIC lexeme (constant, keyword, identifier etc)
   * @param ptype Lexeme type
   * @param psubtype Lexeme subtype
   * @param pname Lexeme name
   * @param pvalue Lexeme value
   */
  Lexeme(LexemeType ptype, LexemeSubType psubtype, string pname, string pvalue);
};

/***
 * @class LexerLine
 * @brief It represents a set of MSX BASIC lexemes forming a line.
 */
class LexerLine {
 private:
  int lexemeIndex;
  vector<Lexeme*> lexemes;
  stack<int> lexemeStack;

  /***
   * @brief Check if the character is numeric
   * @param c Character
   * @return True or false
   */
  bool isNumeric(char c);

  /***
   * @brief Check if the character is a decimal point
   * @param c Character
   * @return True or false
   */
  bool isDecimal(char c);

  /***
   * @brief Check if the character is hexadecimal
   * @param c Character
   * @return True or false
   */
  bool isHexDecimal(char c);

  /***
   * @brief Check if the character is a separator
   * @param c Character
   * @return True or false
   */
  bool isSeparator(char c);

  /***
   * @brief Check if the character is an operator
   * @param c Character
   * @return True or false
   */
  bool isOperator(char c);

  /***
   * @brief Check if the character is an identifier
   * @param c Character
   * @param start Is it the first character from the string?
   * @return True or false
   */
  bool isIdentifier(char c, bool start);

  /***
   * @brief Check if the character is in a commentary string
   * @param c Character
   * @return True or false
   */
  bool isComment(char c);

 public:
  string line;

  /***
   * @brief Evaluates the current line by performing a lexical analysis on it
   * @return True, if a valid line
   */
  bool evaluate();

  /***
   * @brief Print to the terminal the lexemes from the current line
   */
  void print();

  /***
   * @brief Clear the current line lexeme list
   */
  void clearLexemes();

  /***
   * @brief Add a lexeme to the line list
   * @param lexeme Lexeme object
   */
  void addLexeme(Lexeme* lexeme);

  /***
   * @brief Get the first lexeme in the lexeme list
   * @return Lexeme object
   */
  Lexeme* getFirstLexeme();

  /***
   * @brief Get the current lexeme in the lexeme list
   * @return Lexeme object
   */
  Lexeme* getCurrentLexeme();

  /***
   * @brief Get the next lexeme in the lexeme list
   * @return Lexeme object
   */
  Lexeme* getNextLexeme();

  /***
   * @brief Get the previous lexeme in the lexeme list
   * @return Lexeme object
   */
  Lexeme* getPreviousLexeme();

  /***
   * @brief Get the last lexeme in the lexeme list
   * @return Lexeme object
   */
  Lexeme* getLastLexeme();

  /***
   * @brief Get an item in the lexeme list
   * @param i Index
   * @return Lexeme object
   */
  Lexeme* getLexeme(int i);

  /***
   * @brief Set index at before the top of the lexeme list
   */
  void setLexemeBOF();

  /***
   * @brief Get the lexeme list items count
   * @return Lexeme count
   */
  int getLexemeCount();

  /***
   * @brief Push the current lexeme to the stack
   */
  void pushLexeme();

  /***
   * @brief Pop a lexeme from the stack, also setting it as current lexeme
   */
  void popLexeme();

  /***
   * @brief Discard a lexeme from the stack, but keeping the current lexeme
   */
  void popLexemeDiscarding();

  /***
   * @brief LexerLine class constructor.
   * It represents a set of MSX BASIC lexemes forming a line.
   */
  LexerLine();
};

/***
 * @class Lexer
 * @brief Lexer class specialized as a MSX BASIC lexical analyzer
 */
class Lexer {
 public:
  int lineNo = 0;
  vector<LexerLine*> lines;
  string errorMessage;
  BuildOptions* opts;

  /***
   * @brief Clear the lines list
   */
  void clear();

  /***
   * @brief Load a MSX BASIC source code (plain text) in the lines list
   * @param filename Source code file name
   * @return True, if a valid MSX BASIC source code
   */
  bool load(string filename);

  /***
   * @brief Load a MSX BASIC source code (plain text) in the lines list
   * @param opts Build options
   * @return True, if a valid MSX BASIC source code
   */
  bool load(BuildOptions* opts);

  /***
   * @brief Perform a lexical analysis on the lines list
   * @return True, if lexical analysis success
   */
  bool evaluate();

  /***
   * @brief Print the lines list
   */
  void print();

  /***
   * @brief Print to the terminal the invalid line
   */
  void error();

  /***
   * @brief Lexer class constructor, specialized as a MSX BASIC lexical analyzer
   */
  Lexer();
};

#endif  // LEX_H_INCLUDED
