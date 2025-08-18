/***
 * @file tokenizer.h
 * @brief Tokenizer class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 *   https://www.msx.org/wiki/Internal_Structure_Of_BASIC_listing
 *   https://en.wikipedia.org/wiki/Binary-coded_decimal
 *   https://www.msx.org/wiki/BASIC_Routines_In_Main-ROM#Math-pack_routines
 */

#ifndef TOKENIZER_H_INCLUDED
#define TOKENIZER_H_INCLUDED

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <string>
#include <vector>

#include "fswrapper.h"
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

  /***
   * @brief Write a token pcode
   * @param lexeme Lexeme source
   * @param s Destination buffer
   * @param maxlen Buffer maximum length
   * @return pcodes count written
   */
  int writeToken(Lexeme *lexeme, unsigned char *s, int maxlen);

  /***
   * @brief Write a keyword token
   * @param lexeme Lexeme source
   * @param s Destination buffer
   * @param maxlen Buffer maximum length
   * @return pcodes count written
   */
  int writeTokenKeyword(Lexeme *lexeme, unsigned char *s, int maxlen);

  /***
   * @brief Write a literal token
   * @param lexeme Lexeme object
   * @param s Buffer string
   * @param maxlen Buffer maximum length
   * @return pcodes count written
   */
  int writeTokenLiteral(Lexeme *lexeme, unsigned char *s, int maxlen);

  /***
   * @brief Write an operator token
   * @param lexeme Lexeme source
   * @param s Destination buffer
   * @param maxlen Buffer maximum length
   * @return pcodes count written
   */
  int writeTokenOperator(Lexeme *lexeme, unsigned char *s, int maxlen);

  /***
   * @brief Write a string token
   * @param lexeme Lexeme source
   * @param s Destination buffer
   * @param maxlen Buffer maximum length
   * @return pcodes count written
   */
  int writeTokenText(Lexeme *lexeme, unsigned char *s, int maxlen);

  /***
   * @brief Load an additional source code
   * @param lexeme Lexeme source
   * @return True if success
   */
  bool loadInclude(Lexeme *lexeme);

  /***
   * @brief Convert a double to BCD format
   * @param value Value to convert
   * @param words 32 bits destination buffer
   * @note
   * [MSX BASIC math
   * pack](https://www.msx.org/wiki/BASIC_Routines_In_Main-ROM#Math-pack_routines)
   */
  void double2BCD(double value, int *words);

  /***
   * @brief Convert a float to BCD format
   * @param value Value to convert
   * @param words 16 bits destination buffer
   * @note
   * [MSX BASIC math
   * pack](https://www.msx.org/wiki/BASIC_Routines_In_Main-ROM#Math-pack_routines)
   */
  void float2BCD(float value, int *words);

  /***
   * @brief Get the logarithm base 10 from a value
   * @param x Value to compute
   * @return Log10 value computed
   * @note [Common logarithm](https://en.wikipedia.org/wiki/Common_logarithm)
   */
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
