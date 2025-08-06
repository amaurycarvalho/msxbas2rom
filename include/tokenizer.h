/***
 * @file tokenizer.h
 * @brief Tokenizer class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 *   https://www.msx.org/wiki/Internal_Structure_Of_BASIC_listing
 */

#ifndef TOKENIZER_H_INCLUDED
#define TOKENIZER_H_INCLUDED

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

#include "lex.h"

using namespace std;

/***
 * @class
 * @brief Source code line with its list of tokens
 */
class TokenLine {
 public:
  int address;
  int next;
  int number;
  int length;
  unsigned char data[254];
  string text;
  bool goto_gosub;
};

/***
 * @class
 * @brief Tokenizer class specialized as a MSX BASIC pcode generator
 */
class Tokenizer {
 private:
  /***
   * @brief Evaluates one line of code
   * @param lexerline Lexed line of code
   * @return True, if a valid line
   */
  bool evalLine(LexerLine *lexerLine);
  int writeToken(Lexeme *lexeme, unsigned char *s);
  int writeTokenKeyword(Lexeme *lexeme, unsigned char *s);
  int writeTokenLiteral(Lexeme *lexeme, unsigned char *s);
  int writeTokenOperator(Lexeme *lexeme, unsigned char *s);
  int writeTokenText(Lexeme *lexeme, unsigned char *s);
  bool loadInclude(Lexeme *lexeme);

  void double2BCD(double value, int *words);
  void float2BCD(float value, int *words);
  double log10(double x);

  LexerLine *error_line;
  string error_message;
  bool remark, goto_gosub, data, put, resume, restore;
  bool call_cmd, call_cmd_parm;
  bool cmd_cmd, cmd_parm;
  bool skip_next_bracket;

  static const char *keywords_text[];
  static const unsigned char keywords_token[];

 public:
  bool turbo_mode, cmd, force_turbo, pt3, font, open_cmd, turbo_on;
  int lineNo = 0;
  vector<TokenLine *> lines;
  vector<Lexeme *> resourceList;

  /***
   * @brief Evaluates the lexed source code by performing
   * a syntatic analysis on it, identifying the correspondent
   * MSX BASIC tokens in the process
   * @return True, if a valid line
   */
  bool evaluate(Lexer *lexer);

  /***
   * @brief Print to the terminal the lines from the current source code
   */
  void print();

  /***
   * @brief Print to the terminal the invalid line code
   */
  void error();

  bool debug;
};

#endif  // PARSE_H_INCLUDED
