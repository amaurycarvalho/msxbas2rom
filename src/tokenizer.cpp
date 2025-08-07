/***
 * @file tokenizer.cpp
 * @brief Tokenizer class implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 *   https://www.msx.org/wiki/Internal_Structure_Of_BASIC_listing
 */

#include "tokenizer.h"

bool Tokenizer::evaluate(Lexer *lexer) {
  int i, t = lexer->lines.size();
  LexerLine *lexerLine;

  turbo_mode = false;
  turbo_on = false;
  font = false;
  cmd = false;
  pt3 = false;
  open_cmd = false;

  lines.clear();

  for (i = 0, lineNo = 1; i < t; i++, lineNo++) {
    lexerLine = lexer->lines[i];
    if (lexerLine->getLexemeCount() > 0) {
      if (!evalLine(lexerLine)) return false;
    }
  }

  return true;
}

bool Tokenizer::evalLine(LexerLine *lexerLine) {
  Lexeme *lexeme = lexerLine->getFirstLexeme();
  TokenLine *tokenLine;
  unsigned char *s;
  int length, maxlen;

  error_line = lexerLine;
  remark = false;
  goto_gosub = false;
  data = false;
  put = false;
  restore = false;
  resume = false;
  cmd_cmd = false;
  cmd_parm = false;
  call_cmd = false;
  call_cmd_parm = false;
  skip_next_bracket = false;

  if (lexeme) {
    if (lexeme->type == Lexeme::type_literal &&
        lexeme->subtype == Lexeme::subtype_numeric) {
      if (debug) printf("line: %s\n", lexerLine->line.c_str());

      tokenLine = new TokenLine();  // register line number tag
      try {
        tokenLine->number = stoi(lexeme->value);
      } catch (exception &e) {
        printf("Warning: error while converting numeric constant %s\n",
               lexeme->value.c_str());
        tokenLine->number = 0;
      }
      tokenLine->text = lexerLine->line;
      tokenLine->goto_gosub = false;
      tokenLine->address = 0;
      tokenLine->next = 0;
      lines.push_back(tokenLine);

      tokenLine->length = 0;
      s = tokenLine->data;
      maxlen = sizeof(tokenLine->data);
      s[0] = 0;

      while ((lexeme = lexerLine->getNextLexeme())) {
        if (lexeme->value == "(" && skip_next_bracket) {
          skip_next_bracket = false;
          continue;
        }
        length = writeToken(lexeme, s, maxlen);
        if (length < 0) {
          continue;
        } else if (length == 0) {
          error_message = "Line contain invalid token";
          return false;
        }
        if (goto_gosub) tokenLine->goto_gosub = true;
        s += length;
        maxlen -= length;
        tokenLine->length += length;
        s[0] = 0;
        if (tokenLine->length >= 254) {
          error_message = "Line is too long";
          return false;
        }
        if (remark) break;
      }

      if (cmd_cmd && (force_turbo || turbo_on)) {
        s[0] = ',';
        s[1] = '0';
        s[2] = 0;
        tokenLine->length += 2;
      }

      tokenLine->length++;

      return true;

    } else if (lexeme->type == Lexeme::type_keyword) {
      if (lexeme->value == "FILE") {
        if ((lexeme = lexerLine->getNextLexeme())) {
          if (lexeme->type == Lexeme::type_literal &&
              lexeme->subtype == Lexeme::subtype_string) {
            lexeme->name = "FILE";
            resourceList.push_back(lexeme);
            return true;
          } else {
            error_message = "Invalid parameter in FILE keyword";
            return false;
          }
        }
      } else if (lexeme->value == "TEXT") {
        if ((lexeme = lexerLine->getNextLexeme())) {
          if (lexeme->type == Lexeme::type_literal &&
              lexeme->subtype == Lexeme::subtype_string) {
            lexeme->name = "TEXT";
            resourceList.push_back(lexeme);
            return true;
          } else {
            error_message = "Invalid parameter in TEXT keyword";
            return false;
          }
        }

      } else if (lexeme->value == "INCLUDE") {
        if ((lexeme = lexerLine->getNextLexeme())) {
          if (lexeme->type == Lexeme::type_literal &&
              lexeme->subtype == Lexeme::subtype_string) {
            if (!loadInclude(lexeme)) {
              error_message =
                  "INCLUDE file not found or with content syntax error";
              return false;
            }

            return true;

          } else {
            error_message = "Invalid parameter in INCLUDE keyword";
            return false;
          }
        }
      }
    }
  }

  error_message = "Line number is missing";
  return false;
}

bool Tokenizer::loadInclude(Lexeme *lexeme) {
  FILE *file;
  char filename[255], *s;
  char line[255];
  int len = 255, tt, ii, k;
  LexerLine *lexerLine;

  tt = lexeme->value.size();
  s = (char *)lexeme->value.c_str();
  k = 0;

  for (ii = 0; ii < tt; ii++) {
    if (s[ii] != '"') {
      filename[k] = s[ii];
      k++;
    }
  }
  filename[k] = 0;

  /// @remarks
  /// instanciate a new LexerLine
  /// read the file, line by line
  /// evaluate it lexing tokens
  /// and process it by calling evalLine(LexerLine)

  if ((file = fopen(filename, "r"))) {
    bool result = true;

    lexerLine = new LexerLine();

    while (fgets(line, len, file)) {
      lexerLine->line = line;
      if (lexerLine->evaluate())
        evalLine(lexerLine);
      else {
        result = false;
        break;
      }
    }

    fclose(file);
    delete lexerLine;

    return result;
  }
  return false;
}

int Tokenizer::writeToken(Lexeme *lexeme, unsigned char *s, int maxlen) {
  int result, i;

  if (lexeme->type == Lexeme::type_separator && lexeme->value == ":") {
    goto_gosub = false;
  }

  if (call_cmd_parm) {
    if (lexeme->type == Lexeme::type_separator && lexeme->value == ":") {
      call_cmd = false;
      call_cmd_parm = false;
    } else
      return writeTokenText(lexeme, s, maxlen);
  }

  if (cmd_cmd) {
    if (lexeme->type == Lexeme::type_separator && lexeme->value == ":") {
      cmd_cmd = false;
      cmd_parm = false;
      if (force_turbo || turbo_on) {
        error_message =
            "Additional instructions not allowed in front of CMD in Turbo mode";
        return false;
      }
    } else {
      if (force_turbo || turbo_on) {
        if (lexeme->type == Lexeme::type_identifier) {
          cmd_parm = true;
          result = sprintf((char *)s, ",%i,%s", '@', lexeme->value.c_str());
        } else if (lexeme->type == Lexeme::type_literal) {
          cmd_parm = true;
          try {
            i = stoi(lexeme->value.c_str());
          } catch (exception &e) {
            printf("Warning: error while converting numeric constant %s\n",
                   lexeme->value.c_str());
            i = 0;
          }
          result =
              sprintf((char *)s, ",%i,%i,%i", '#', i & 0xFF, i >> 8 & 0xFF);
        } else if (lexeme->type == Lexeme::type_keyword) {
          writeTokenKeyword(lexeme, s, maxlen);
          i = s[0];
          result = sprintf((char *)s, ",%i", i);
        } else {
          result = -1;  // ignore this token
        }
        return result;
      }
    }
  }

  if (data) {
    result = writeTokenText(lexeme, s, maxlen);
  } else {
    if (lexeme->type == Lexeme::type_keyword)
      result = writeTokenKeyword(lexeme, s, maxlen);
    else if (lexeme->type == Lexeme::type_literal)
      result = writeTokenLiteral(lexeme, s, maxlen);
    else if (lexeme->type == Lexeme::type_operator)
      result = writeTokenOperator(lexeme, s, maxlen);
    else {
      result = writeTokenText(lexeme, s, maxlen);
    }
  }

  return result;
}

int Tokenizer::writeTokenKeyword(Lexeme *lexeme, unsigned char *s, int maxlen) {
  int result = 0, i;
  char *ss;
  string value;

  // goto_gosub = false;
  data = false;
  restore = false;
  resume = false;

  // keywords

  if (lexeme->subtype == Lexeme::subtype_function) {
    value = lexeme->value;
    i = 0;
    while (keywords_text[i]) {
      if (value == keywords_text[i]) {
        s[0] = keywords_token[i];
        result = 1;
        if (value == "INTERVAL") {
          s[0] = 0xFF;
          s[1] = 0x85;
          s[2] = 0x45;
          s[3] = 0x52;
          s[4] = 0xFF;
          s[5] = 0x94;
          result = 6;
        } else if (s[0] < 0x80) {
          // functions
          s[1] = s[0] | 0x80;
          s[0] = 0xFF;
          result = 2;
        } else if (s[0] == 0x85) {
          // input$ case
          s[1] = '$';
          result = 2;
        } else if (s[0] == 0xC7 && !put) {
          // sprite case
          s[1] = '$';
          result = 2;
        } else if (s[0] == 0xDF) {
          // spc( case
          skip_next_bracket = true;
        } else if (s[0] == 0xDB) {
          // tab( case
          skip_next_bracket = true;
        }
        break;
      }
      i++;
    }

  } else {
    if (call_cmd) {
      if (lexeme->value == "TURBO") {
        turbo_mode = true;
        turbo_on = !turbo_on;
      }
      result = writeTokenText(lexeme, s, maxlen);
      if (!call_cmd_parm) {
        s[result] = 32;
        s[result + 1] = 0;
        result++;
        call_cmd_parm = true;
      }
    }

    if (cmd) {
      if (lexeme->value == "SETFNT") {
        font = true;
      } else {
        ss = (char *)lexeme->value.c_str();
        if (ss[0] == 'P' && ss[1] == 'T' && ss[2] == '3') pt3 = true;
      }
    }

    put = false;

    if (lexeme->value == "OPEN") open_cmd = true;

    if (lexeme->value == "CALL") {
      call_cmd = true;
    }

    if (lexeme->value == "CMD") {
      cmd = true;
      cmd_cmd = true;
      if (turbo_on || force_turbo) {
        s[0] = ':';
        s[1] = 143;
        s[2] = 230;  // remark quote
        s[3] = '#';
        s[4] = 'I';  // include assembly (xbasic)
        s[5] = 32;
        s[6] = '2';
        s[7] = '0';
        s[8] = '5';
        s[9] = ',';
        s[10] = '1';
        s[11] = '9';
        s[12] = ',';
        s[13] = '1';
        s[14] = '2';
        s[15] = '8';  // call cmd_parse.xbasic (=CD 80 13)
        result = 16;
        return result;
      }
    }

    if (lexeme->value == "TPSET") {
      s[0] = 'T';
      s[1] = 0xC2;
      result = 2;
      return result;
    } else if (lexeme->value == "TAND") {
      s[0] = 0xFF;
      s[1] = 0x8D;
      s[2] = 0x44;
      result = 3;
      return result;
    } else if (lexeme->value == "TOR") {
      s[0] = 0xD9;
      s[1] = 0x52;
      result = 2;
      return result;
    } else if (lexeme->value == "TPRESET") {
      s[0] = 'T';
      s[1] = 0xC3;
      result = 2;
      return result;
    } else if (lexeme->value == "TXOR") {
      s[0] = 'T';
      s[1] = 0xF8;
      result = 2;
      return result;
    }

    value = lexeme->value;
    i = 0;
    while (keywords_text[i]) {
      if (value == keywords_text[i]) {
        s[0] = keywords_token[i];
        result = 1;
        if (!cmd_cmd) {
          if (value == "INTERVAL") {
            s[0] = 0xFF;
            s[1] = 0x85;
            s[2] = 0x45;
            s[3] = 0x52;
            s[4] = 0xFF;
            s[5] = 0x94;
            result = 6;
          } else if (value == "RESTORE") {
            restore = true;
          } else if (value == "RETURN") {
            goto_gosub = true;
          } else if (value == "RESUME") {
            resume = true;
          } else if (s[0] < 0x80) {
            // functions
            s[1] = s[0] | 0x80;
            s[0] = 0xFF;
            result = 2;
          } else if (s[0] == 0xA1 || s[0] == 0x8F) {
            // else or rem
            if (s[0] == 0x8F) remark = true;
            s[1] = s[0];
            s[0] = ':';
            result = 2;
            goto_gosub = false;
          } else if (s[0] == 0x89 || s[0] == 0x8D) {
            // goto and gosub cases
            goto_gosub = true;
          } else if (s[0] == 0xDA || s[0] == 0xA1) {
            // then or else cases
            goto_gosub = true;
          } else if (s[0] == 0x84) {
            // data case
            data = true;
          } else if (s[0] == 0xB3) {
            // put case
            put = true;
          } else if (s[0] == 0xC1 && s[1] == '(') {
            // play function case
            if (turbo_on || force_turbo) {
              s[0] = 0xDD;  // USR2
              s[1] = 19;    // number 2 coded
              result = 2;
            }
          }
        }
        break;
      }
      i++;
    }
  }

  return result;
}

int Tokenizer::writeTokenLiteral(Lexeme *lexeme, unsigned char *s, int maxlen) {
  int result = 0, ivalue;
  double dvalue;
  int words[4];
  char *ss;

  if (data) {
    result = writeTokenText(lexeme, s, maxlen);
  } else if (lexeme->subtype == Lexeme::subtype_single_decimal) {
    try {
      dvalue = stod(lexeme->value);
    } catch (exception &e) {
      printf("Warning: error while converting numeric constant %s\n",
             lexeme->value.c_str());
      dvalue = 0;
    }
    float2BCD(dvalue, &words[0]);
    s[0] = 0x1D;
    s[1] = words[0] & 0xFF;
    s[2] = words[0] >> 8;
    s[3] = words[1] & 0xFF;
    s[4] = words[1] >> 8;
    result = 5;
  } else if (lexeme->subtype == Lexeme::subtype_double_decimal) {
    try {
      dvalue = stod(lexeme->value);
    } catch (exception &e) {
      printf("Warning: error while converting numeric constant %s\n",
             lexeme->value.c_str());
      dvalue = 0;
    }
    double2BCD(dvalue, &words[0]);
    s[0] = 0x1F;
    s[1] = words[0] & 0xFF;
    s[2] = words[0] >> 8;
    s[3] = words[1] & 0xFF;
    s[4] = words[1] >> 8;
    s[5] = words[2] & 0xFF;
    s[6] = words[2] >> 8;
    s[7] = words[3] & 0xFF;
    s[8] = words[3] >> 8;
    result = 9;
  } else if (lexeme->subtype == Lexeme::subtype_numeric) {
    ss = (char *)lexeme->value.c_str();
    if (ss[0] == '&') {
      if (ss[1] == 'B' || ss[1] == 'b') {
        ss[1] = 'B';  // force uppercase
        result = writeTokenText(lexeme, s, maxlen);
      } else if (ss[1] == 'O' || ss[1] == 'o') {
        try {
          ivalue = stoi(&ss[2], 0, 8);
        } catch (exception &e) {
          printf("Warning: error while converting numeric constant %s\n",
                 lexeme->value.c_str());
          ivalue = 0;
        }
        s[0] = 0x0B;
        s[1] = ivalue & 0xFF;
        s[2] = ivalue >> 8;
        result = 3;
      } else if (ss[1] == 'H' || ss[1] == 'h') {
        try {
          ivalue = stoi(&ss[2], 0, 16);
        } catch (exception &e) {
          printf("Warning: error while converting numeric constant %s\n",
                 lexeme->value.c_str());
          ivalue = 0;
        }
        s[0] = 0x0C;
        s[1] = ivalue & 0xFF;
        s[2] = ivalue >> 8;
        result = 3;
      }
    } else {
      try {
        ivalue = stoi(lexeme->value);
      } catch (exception &e) {
        printf("Warning: error while converting numeric constant %s\n",
               lexeme->value.c_str());
        ivalue = 0;
      }
      if (ivalue < 0) ivalue = -ivalue;
      if (goto_gosub || restore || resume) {
        s[0] = 0x0E;
        s[1] = ivalue & 0xFF;
        s[2] = ivalue >> 8;
        result = 3;
        // goto_gosub = false;
      } else if (ivalue >= 0 && ivalue < 10) {
        s[0] = 0x11 + ivalue;
        result = 1;
      } else if (ivalue <= 255) {
        s[0] = 0x0F;
        s[1] = ivalue;
        result = 2;
      } else {
        s[0] = 0x1C;
        s[1] = ivalue & 0xFF;
        s[2] = ivalue >> 8;
        result = 3;
      }
    }
  } else {
    result = writeTokenText(lexeme, s, maxlen);
  }
  return result;
}

int Tokenizer::writeTokenOperator(Lexeme *lexeme, unsigned char *s,
                                  int maxlen) {
  int result = 0;
  if (lexeme->subtype == Lexeme::subtype_boolean_operator) {
    result = writeTokenKeyword(lexeme, s, maxlen);
  } else if (lexeme->value == "'") {
    s[0] = ':';
    s[1] = 0x8F;  // rem
    s[2] = 0xE6;
    result = 3;
    // remark = true;
  } else if (lexeme->value == "=") {
    s[0] = 0xEF;
    result = 1;
  } else if (lexeme->value == "+") {
    s[0] = 0xF1;
    result = 1;
  } else if (lexeme->value == "*") {
    s[0] = 0xF3;
    result = 1;
  } else if (lexeme->value == "^") {
    s[0] = 0xF5;
    result = 1;
  } else if (lexeme->value == ">") {
    s[0] = 0xEE;
    result = 1;
  } else if (lexeme->value == "<") {
    s[0] = 0xF0;
    result = 1;
  } else if (lexeme->value == "-") {
    s[0] = 0xF2;
    result = 1;
  } else if (lexeme->value == "/") {
    s[0] = 0xF4;
    result = 1;
  } else if (lexeme->value == "\\") {
    s[0] = 0xFC;
    result = 1;
  } else {
    result = writeTokenText(lexeme, s, maxlen);
  }
  return result;
}

int Tokenizer::writeTokenText(Lexeme *lexeme, unsigned char *s, int maxlen) {
  strlcpy((char *)s, lexeme->value.c_str(), maxlen);
  return lexeme->value.size();
}

void Tokenizer::print() {
  TokenLine *line;
  for (unsigned int i = 0; i < lines.size(); i++) {
    line = lines[i];
    if (line)
      printf("%i (%i bytes) = %s\n", line->number, line->length,
             line->text.c_str());
  }
}

void Tokenizer::error() {
  if (error_line) error_line->print();
  if (error_message.size() > 0) printf("%s\n", error_message.c_str());
}

//----------------------------------------------------------------------------------------------

void Tokenizer::double2BCD(double value, int *words) {
  int exponent, digit, i, k, neg;
  double mantissa;
  unsigned char digits[14], *p;
  for (i = 0; i < 4; i++) words[i] = 0;
  for (i = 0; i < 14; i++) digits[i] = 0;
  neg = 64;
  if (value < 0) {
    neg += 128;  // binary 10000000
    value = -value;
  }
  exponent = (int)log10(value);
  exponent++;
  mantissa = pow(10, exponent);
  mantissa = value / mantissa;
  i = 0;
  while (mantissa > 0 && i < 14) {
    mantissa *= 10;
    digit = (int)mantissa;
    digits[i] = (unsigned char)digit;
    mantissa -= digit;
    i++;
  }
  p = (unsigned char *)&words[0];  // word 0
  p[0] = neg + exponent;
  p[1] = digits[0] * 16 + digits[1];

  for (i = 1, k = 2; i < 4; i++, k += 4) {
    p = (unsigned char *)&words[i];  // word i
    p[0] = digits[k] * 16 + digits[k + 1];
    p[1] = digits[k + 2] * 16 + digits[k + 3];
  }
}

void Tokenizer::float2BCD(float value, int *words) {
  double d_value = value;
  int d_words[4];
  double2BCD(d_value, (int *)&d_words);
  words[0] = d_words[0];
  words[1] = d_words[1];
}

double Tokenizer::log10(double x) {
  return log(x) / log(10.0);
}

//----------------------------------------------------------------------------------------------

const char *Tokenizer::keywords_text[] = {
    "ABS",       "AND",       "ASC",       "ATN",       "ATTR$",   "AUTO",
    "BASE",      "BEEP",      "BIN$",      "BLOAD",     "BSAVE",   "CALL",
    "CDBL",      "CHR$",      "CINT",      "CIRCLE",    "CLEAR",   "CLOAD",
    "CLOSE",     "CLS",       "CMD",       "COLOR",     "COS",     "CONT",
    "COPY",      "CSAVE",     "CSNG",      "CSRLIN",    "CVD",     "CVI",
    "CVS",       "DATA",      "DEF",       "DEFDBL",    "DEFINT",  "DEFSNG",
    "DEFSTR",    "DELETE",    "DIM",       "DRAW",      "DSKF",    "DSKI$",
    "DSKO$",     "ELSE",      "END",       "EOF",       "EQV",     "ERASE",
    "ERL",       "ERR",       "ERROR",     "EXP",       "FIELD",   "FILES",
    "FIX",       "FN",        "FOR",       "FPOS",      "FRE",     "GET",
    "GOSUB",     "GOTO",      "GO TO",     "HEX$",      "IF",      "IMP",
    "INKEY$",    "INP",       "INPUT",     "INSTR",     "INT",     "IPL",
    "KEY",       "KILL",      "LEFT$",     "LEN",       "LET",     "LFILES",
    "LINE",      "LIST",      "LLIST",     "LOAD",      "LOC",     "LOCATE",
    "LOF",       "LOG",       "LPOS",      "LPRINT",    "LSET",    "MAX",
    "MERGE",     "MID$",      "MKD$",      "MKI$",      "MKS$",    "MOD",
    "MOTOR",     "NAME",      "NEW",       "NEXT",      "NOT",     "OCT$",
    "OFF",       "ON",        "OPEN",      "OR",        "OUT",     "PAD",
    "PAINT",     "PDL",       "PEEK",      "PLAY",      "POINT",   "POKE",
    "POS",       "PRESET",    "PRINT",     "PSET",      "PUT",     "READ",
    "REM",       "RENUM",     "RESTORE",   "RESUME",    "RETURN",  "RIGHT$",
    "RND",       "RSET",      "RUN",       "SAVE",      "SCREEN",  "SET",
    "SGN",       "SIN",       "SOUND",     "SPACE$",    "SPC",     "SPRITE",
    "SQR",       "STEP",      "STICK",     "STOP",      "STR$",    "STRIG",
    "STRING$",   "SWAP",      "TAB",       "TAN",       "THEN",    "TIME",
    "TO",        "TROFF",     "TRON",      "USING",     "USR",     "VAL",
    "VARPTR",    "VDP",       "VPEEK",     "VPOKE",     "WAIT",    "WIDTH",
    "XOR",       "INPUT$",    "SPRITE$",   "?",         "_",       "INTERVAL",
    "RANDOMIZE", "SHL",       "SHR",       "INKEY",     "RUNASM",  "RUNBAS",
    "WRTVRAM",   "WRTFNT",    "WRTCHR",    "WRTCLR",    "WRTSCR",  "WRTSPRPAT",
    "WRTSPRCLR", "WRTSPRATR", "RAMTOVRAM", "VRAMTORAM", "DISSCR",  "ENASCR",
    "KEYCLKOFF", "MUTE",      "PT3LOAD",   "PT3PLAY",   "PT3MUTE", "SETFNT",
    "CLRSCR",    "RAMTORAM",  "PT3LOOP",   "PT3REPLAY", "CLRKEY",  0};

const unsigned char Tokenizer::keywords_token[] = {
    0x06, 0xF6, 0x15, 0x0E, 0xE9, 0xA9, 0xC9, 0xC0, 0x1D, 0xCF, 0xD0, 0xCA,
    0x20, 0x16, 0x1E, 0xBC, 0x92, 0x9B, 0xB4, 0x9F, 0xD7, 0xBD, 0x0C, 0x99,
    0xD6, 0x9A, 0x1F, 0xE8, 0x2A, 0x28, 0x29, 0x84, 0x97, 0xAE, 0xAC, 0xAD,
    0xAB, 0xA8, 0x86, 0xBE, 0x26, 0xEA, 0xD1, 0xA1, 0x81, 0x2B, 0xF9, 0xA5,
    0xE1, 0xE2, 0xA6, 0x0B, 0xB1, 0xB7, 0x21, 0xDE, 0x82, 0x27, 0x0F, 0xB2,
    0x8D, 0x89, 0x89, 0x1B, 0x8B, 0xFA, 0xEC, 0x10, 0x85, 0xE5, 0x05, 0xD5,
    0xCC, 0xD4, 0x01, 0x12, 0x88, 0xBB, 0xAF, 0x93, 0x9E, 0xB5, 0x2C, 0xD8,
    0x2D, 0x0A, 0x1C, 0x9D, 0xB8, 0xCD, 0xB6, 0x03, 0x30, 0x2E, 0x2F, 0xFB,
    0xCE, 0xD3, 0x94, 0x83, 0xE0, 0x1A, 0xEB, 0x95, 0xB0, 0xF7, 0x9C, 0x25,
    0xBF, 0x24, 0x17, 0xC1, 0xED, 0x98, 0x11, 0xC3, 0x91, 0xC2, 0xB3, 0x87,
    0x8F, 0xAA, 0x8C, 0xA7, 0x8E, 0x02, 0x08, 0xB9, 0x8A, 0xBA, 0xC5, 0xD2,
    0x04, 0x09, 0xC4, 0x19, 0xDF, 0xC7, 0x07, 0xDC, 0x22, 0x90, 0x13, 0x23,
    0xE3, 0xA4, 0xDB, 0x0D, 0xDA, 0xCB, 0xD9, 0xA3, 0xA2, 0xE4, 0xDD, 0x14,
    0xE7, 0xC8, 0x18, 0xC6, 0x96, 0xA0, 0xF8, 0x85, 0xC7, 0x91, 0xCA, 0xFF,
    0x8F, 0x8F, 0x8F, 0x8F, 'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',
    'i',  'j',  'k',  'l',  'm',  'n',  'o',  'p',  'q',  'r',  's',  't',
    'u',  'v',  'w',  'x',  'y',  0};
