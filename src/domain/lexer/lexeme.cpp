/***
 * @file lexeme.cpp
 * @brief MSX BASIC lexeme class implementation
 * @author Amaury Carvalho (2019-2025)
 */

#include "lexeme.h"

#include <string>

/***
 * @name Lexeme class functions
 */

Lexeme::Lexeme() {
  clear();
}

Lexeme::Lexeme(Lexeme* plexeme) {
  clear();
  type = plexeme->type;
  subtype = plexeme->subtype;
  name = plexeme->name;
  value = plexeme->value;
  tag = plexeme->tag;
  isArray = plexeme->isArray;
  isAbstract = plexeme->isAbstract;
  isUnary = plexeme->isUnary;
  indent = plexeme->indent;
  y_factor = plexeme->y_factor;
  x_factor = plexeme->x_factor;
  y_size = plexeme->y_size;
  x_size = plexeme->x_size;
  array_size = plexeme->array_size;
  parm_count = plexeme->parm_count;
}

Lexeme::Lexeme(LexemeType ptype, LexemeSubType psubtype, string pname) {
  clear();
  type = ptype;
  subtype = psubtype;
  name = pname;
  value = pname;
}

Lexeme::Lexeme(LexemeType ptype, LexemeSubType psubtype, string pname,
               string pvalue) {
  clear();
  type = ptype;
  subtype = psubtype;
  name = pname;
  value = pvalue;
}

Lexeme* Lexeme::clone() {
  return new Lexeme(this);
}

void Lexeme::clear() {
  type = Lexeme::type_unknown;
  subtype = Lexeme::subtype_any;
  name = "";
  value = "";
  tag = "";
  isArray = false;
  isAbstract = false;
  isUnary = false;
  indent = 0;
  y_factor = 0;
  x_factor = 0;
  y_size = 0;
  x_size = 0;
  array_size = 0;
  parm_count = 0;
}

string Lexeme::toString(int indentOverride) {
  int textIndent = indent;
  if (indentOverride >= 0) textIndent = indentOverride;

  string out;
  out.append(textIndent, ' ');
  out += "--> ";
  out += getTypeName();
  out += " ";
  out += getSubTypeName();
  if (isArray) out += " (array)";
  out += ": ";
  out += value;
  out += "\n";
  return out;
}

char* Lexeme::getTypeName() {
  static char LexemeTypeName[][20] = {"Unknown",   "Identifier", "Keyword",
                                      "Separator", "Operator",   "Literal",
                                      "Comment"};

  return LexemeTypeName[type];
}

char* Lexeme::getSubTypeName() {
  static char LexemeSubTypeName[][20] = {"",
                                         "String",
                                         "Numeric",
                                         "Single Decimal",
                                         "Double Decimal",
                                         "Basic String",
                                         "Boolean/Remainder",
                                         "Function",
                                         "Null",
                                         "Binary Data",
                                         "Integer Data",
                                         "Unknown"};

  return LexemeSubTypeName[subtype];
}

bool Lexeme::isKeyword(string pvalue) {
  return (type == Lexeme::type_keyword && value == pvalue);
}

bool Lexeme::isSeparator(string pvalue) {
  return (type == Lexeme::type_separator && value == pvalue);
}

bool Lexeme::isOperator(string pvalue) {
  return (type == Lexeme::type_operator && value == pvalue);
}

bool Lexeme::isLiteralNumeric() {
  return (type == Lexeme::type_literal && subtype == Lexeme::subtype_numeric);
}

bool Lexeme::isKeyword() {
  return (
      value == "CLS" || value == "PRINT" || value == "END" || value == "GOTO" ||
      value == "IF" || value == "THEN" || value == "ELSE" || value == "COLOR" ||
      value == "GOSUB" || value == "FOR" || value == "NEXT" ||
      value == "RETURN" || value == "ON" || value == "OFF" || value == "STOP" ||
      value == "MOD" || value == "CALL" || value == "PAUSE" ||
      value == "INTERVAL" || value == "ERROR" || value == "FONT" ||
      value == "GET" || value == "DATE" || value == "TIME" || value == "SET" ||
      value == "ASC" || value == "BIN$" || value == "CDBL" || value == "CHR$" ||
      value == "CINT" || value == "CSNG" || value == "HEX$" ||
      value == "OCT$" || value == "VAL" || value == "WIDTH" || value == "SPC" ||
      value == "STR$" || value == "BLOAD" || value == "BSAVE" ||
      value == "CLOAD" || value == "CSAVE" || value == "CLOSE" ||
      value == "EOF" || value == "LOAD" || value == "MAX" || value == "FILES" ||
      value == "LEN" || value == "USING$" || value == "MERGE" ||
      value == "MOTOR" || value == "OPEN" || value == "RUN" ||
      value == "SAVE" || value == "VARPTR" || value == "NEW" ||
      value == "USING" || value == "BASE" || value == "CIRCLE" ||
      value == "COPY" || value == "SCREEN" || value == "CSRLIN" ||
      value == "DRAW" || value == "LINE" || value == "LOCATE" ||
      value == "LPOS" || value == "PAINT" || value == "POINT" ||
      value == "POS" || value == "PRESET" || value == "PSET" ||
      value == "DRAW" || value == "INP" || value == "PEEK" || value == "POKE" ||
      value == "VPEEK" || value == "VPOKE" || value == "IPEEK" ||
      value == "IPOKE" || value == "BASE" || value == "VDP" ||
      value == "DEFINT" || value == "DEFDBL" || value == "DEFSNG" ||
      value == "DEFSTR" || value == "DEF" || value == "FN" || value == "OUT" ||
      value == "WAIT" || value == "INPUT$" || value == "INPUT" ||
      value == "KEY" || value == "STRIG" || value == "STEP" || value == "PAD" ||
      value == "PDL" || value == "STICK" || value == "AND" || value == "EQV" ||
      value == "IMP" || value == "NOT" || value == "OR" || value == "XOR" ||
      value == "ABS" || value == "CDBL" || value == "CINT" || value == "CSNG" ||
      value == "EXP" || value == "FIX" || value == "INT" || value == "LOG" ||
      value == "RND" || value == "SGN" || value == "SQR" || value == "DEF" ||
      value == "USR" || value == "LLIST" || value == "LPRINT" ||
      value == "BEEP" || value == "PLAY" || value == "SOUND" ||
      value == "SPRITE$" || value == "SPRITE" || value == "INSTR" ||
      value == "LEFT$" || value == "MID$" || value == "RIGHT$" ||
      value == "SPACE$" || value == "STRING$" || value == "ATN" ||
      value == "COS" || value == "SIN" || value == "TAN" || value == "CLEAR" ||
      value == "DATA" || value == "DIM" || value == "ERASE" || value == "LET" ||
      value == "REDIM" || value == "TO" || value == "AND" || value == "TAB" ||
      value == "REM" || value == "READ" || value == "RESTORE" ||
      value == "RUN" || value == "SWAP" || value == "SEED" ||
      value == "RANDOMIZE" || value == "RESUME" || value == "SHR" ||
      value == "SHL" || value == "INKEY$" || value == "INKEY" ||
      value == "KANJI" || value == "PUT" || value == "TPSET" ||
      value == "VIDEO" || value == "TAND" || value == "TOR" ||
      value == "TPRESET" || value == "TXOR" || value == "PAGE" ||
      value == "SCROLL" || value == "FRE" || value == "HEAP" ||
      value == "TILE" || value == "TILES" || value == "MSX" ||
      value == "RESOURCE" || value == "RESOURCESIZE" || value == "INCLUDE" ||
      value == "TURBO" || value == "TEXT" || value == "CMD" ||
      value == "FILE" || value == "RUNASM" || value == "RUNBAS" ||
      value == "WRTVRAM" || value == "WRTCHR" || value == "WRTCLR" ||
      value == "WRTSCR" || value == "WRTSPRPAT" || value == "WRTSPRCLR" ||
      value == "WRTSPRATR" || value == "RAMTOVRAM" || value == "VRAMTORAM" ||
      value == "RAMTORAM" || value == "RSCTORAM" || value == "PT3LOAD" ||
      value == "PT3PLAY" || value == "PT3MUTE" || value == "PT3LOOP" ||
      value == "PT3REPLAY" || value == "PLYLOAD" || value == "PLYSONG" ||
      value == "PLYPLAY" || value == "PLYMUTE" || value == "PLYLOOP" ||
      value == "PLYREPLAY" || value == "PLYSOUND" || value == "PLYSTATUS" ||
      value == "DISSCR" || value == "ENASCR" || value == "WRTFNT" ||
      value == "SETFNT" || value == "?" || value == "_" || value == "CLRSCR" ||
      value == "KEYCLKOFF" || value == "CLRKEY" || value == "COLLISION" ||
      value == "MUTE" || value == "PSG" || value == "NTSC" ||
      value == "IDATA" || value == "IREAD" || value == "IRESTORE" ||
      value == "MAKER" || value == "UPDFNTCLR" || value == "PATTERN" ||
      value == "FLIP" || value == "ROTATE" || value == "FROM" ||
      value == "PASTE" || value == "ADJUST" || value == "TITLE" ||
      value == "PROMPT");
}

bool Lexeme::isBooleanOperator() {
  return (value == "AND" || value == "OR" || value == "XOR" || value == "MOD" ||
          value == "IMP" || value == "EQV" || value == "NOT" ||
          value == "SHR" || value == "SHL");
}

bool Lexeme::isFunction() {
  return (
      value == "DATE" || value == "TIME" || value == "ASC" || value == "BIN$" ||
      value == "CDBL" || value == "CHR$" || value == "SPC" || value == "CINT" ||
      value == "CSNG" || value == "HEX$" || value == "OCT$" || value == "VAL" ||
      value == "EOF" || value == "VARPTR" || value == "STR$" ||
      value == "CSRLIN" || value == "LPOS" || value == "POINT" ||
      value == "POS" || value == "PEEK" || value == "LEN" || value == "FRE" ||
      value == "HEAP" || value == "VPEEK" || value == "INP" ||
      value == "BASE" || value == "VARPTR" || value == "IPEEK" ||
      value == "INKEY" || value == "INPUT$" || value == "INKEY$" ||
      value == "STRIG" || value == "PAD" || value == "PDL" ||
      value == "STICK" || value == "ABS" || value == "CDBL" || value == "VDP" ||
      value == "SNG" || value == "EXP" || value == "FIX" || value == "INT" ||
      value == "DBL" || value == "LOG" || value == "RND" || value == "SGN" ||
      value == "SQR" || value == "SPRITE$" || value == "INSTR" ||
      value == "LEFT$" || value == "MID$" || value == "RIGHT$" ||
      value == "SPACE$" || value == "STRING$" || value == "ATN" ||
      value == "COS" || value == "SIN" || value == "TAN" || value == "TAB" ||
      value == "ATTR$" || value == "USR0" || value == "USR" ||
      value == "USING$" || value == "COLLISION" || value == "TILE" ||
      value == "MSX" || value == "RESOURCE" || value == "RESOURCESIZE" ||
      value == "PSG" || value == "NTSC" || value == "TURBO" ||
      value == "MAKER" || value == "PLYSTATUS");
}
