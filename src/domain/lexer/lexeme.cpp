/***
 * @file lexeme.cpp
 * @brief MSX BASIC lexeme class implementation
 * @author Amaury Carvalho (2019-2025)
 */

#include "lexeme.h"

#include <map>
#include <set>
#include <string>

namespace {

//--------------------------------------------------
// TYPE NAME MAP
//--------------------------------------------------

static const map<Lexeme::LexemeType, const char*> typeNames = {
    {Lexeme::type_unknown, "Unknown"},
    {Lexeme::type_identifier, "Identifier"},
    {Lexeme::type_keyword, "Keyword"},
    {Lexeme::type_separator, "Separator"},
    {Lexeme::type_operator, "Operator"},
    {Lexeme::type_literal, "Literal"},
    {Lexeme::type_comment, "Comment"}};

//--------------------------------------------------
// SUBTYPE NAME MAP
//--------------------------------------------------

static const map<Lexeme::LexemeSubType, const char*> subTypeNames = {
    {Lexeme::subtype_any, ""},
    {Lexeme::subtype_string, "String"},
    {Lexeme::subtype_numeric, "Numeric"},
    {Lexeme::subtype_single_decimal, "Single Decimal"},
    {Lexeme::subtype_double_decimal, "Double Decimal"},
    {Lexeme::subtype_basic_string, "Basic String"},
    {Lexeme::subtype_boolean_operator, "Boolean/Remainder"},
    {Lexeme::subtype_function, "Function"},
    {Lexeme::subtype_null, "Null"},
    {Lexeme::subtype_binary_data, "Binary Data"},
    {Lexeme::subtype_integer_data, "Integer Data"},
    {Lexeme::subtype_unknown, "Unknown"}};

//--------------------------------------------------
// KEYWORD SET
//--------------------------------------------------

const set<string> keywords = {
    "?",         "ABS",       "ADJUST",       "AND",       "ASC",
    "ATN",       "BASE",      "BEEP",         "BIN$",      "BLOAD",
    "BSAVE",     "CALL",      "CDBL",         "CHR$",      "CINT",
    "CIRCLE",    "CLEAR",     "CLRSCR",       "CLRKEY",    "CLS",
    "CLOAD",     "CLOSE",     "CMD",          "COLLISION", "COLOR",
    "COPY",      "COS",       "CSAVE",        "CSNG",      "CSRLIN",
    "DATA",      "DATE",      "DEF",          "DEFDBL",    "DEFINT",
    "DEFSNG",    "DEFSTR",    "DIM",          "DISSCR",    "DRAW",
    "ELSE",      "ENASCR",    "END",          "EOF",       "EQV",
    "ERASE",     "ERROR",     "EXP",          "FILES",     "FILE",
    "FIX",       "FLIP",      "FN",           "FONT",      "FOR",
    "FROM",      "FRE",       "GET",          "GOSUB",     "GOTO",
    "HEAP",      "HEX$",      "IDATA",        "IF",        "IMP",
    "INCLUDE",   "INKEY",     "INKEY$",       "INP",       "INPUT",
    "INPUT$",    "INSTR",     "INT",          "INTERVAL",  "IPEEK",
    "IPOKE",     "IREAD",     "IRESTORE",     "KANJI",     "KEY",
    "KEYCLKOFF", "LEFT$",     "LEN",          "LET",       "LINE",
    "LLIST",     "LOAD",      "LOCATE",       "LOG",       "LPOS",
    "LPRINT",    "MAKER",     "MAXFILES",     "MERGE",     "MID$",
    "MOD",       "MOTOR",     "MSX",          "MUTE",      "NEW",
    "NEXT",      "NOT",       "NTSC",         "OCT$",      "OFF",
    "ON",        "OPEN",      "OR",           "OUT",       "PAD",
    "PAGE",      "PAINT",     "PASTE",        "PATTERN",   "PAUSE",
    "PDL",       "PEEK",      "PLAY",         "PLYLOAD",   "PLYLOOP",
    "PLYMUTE",   "PLYPLAY",   "PLYREPLAY",    "PLYSOUND",  "PLYSTATUS",
    "PLYSONG",   "POINT",     "POKE",         "POS",       "PRESET",
    "PRINT",     "PROMPT",    "PSG",          "PSET",      "PT3LOAD",
    "PT3LOOP",   "PT3MUTE",   "PT3PLAY",      "PT3REPLAY", "PUT",
    "RANDOMIZE", "RAMTORAM",  "RAMTOVRAM",    "READ",      "REDIM",
    "REM",       "RESOURCE",  "RESOURCESIZE", "RESTORE",   "RESUME",
    "RETURN",    "RIGHT$",    "RND",          "ROTATE",    "RSCTORAM",
    "RUN",       "RUNASM",    "RUNBAS",       "SAVE",      "SCREEN",
    "SCROLL",    "SEED",      "SET",          "SETFNT",    "SGN",
    "SHL",       "SHR",       "SIN",          "SOUND",     "SPACE$",
    "SPC",       "SPRITE",    "SPRITE$",      "SQR",       "STEP",
    "STICK",     "STOP",      "STR$",         "STRING$",   "STRIG",
    "SWAP",      "TAB",       "TAN",          "TAND",      "TEXT",
    "THEN",      "TILE",      "TILES",        "TIME",      "TITLE",
    "TO",        "TOR",       "TPRESET",      "TPSET",     "TURBO",
    "TXOR",      "UPDFNTCLR", "USR",          "USING",     "USING$",
    "VAL",       "VARPTR",    "VDP",          "VIDEO",     "VPEEK",
    "VPOKE",     "VRAMTORAM", "WAIT",         "WIDTH",     "WRTCHR",
    "WRTCLR",    "WRTFNT",    "WRTSCR",       "WRTSPRATR", "WRTSPRCLR",
    "WRTSPRPAT", "WRTVRAM",   "XOR",          "_"};

//--------------------------------------------------
// BOOLEAN OPERATORS
//--------------------------------------------------

const set<string> booleanOps = {"AND", "EQV", "IMP", "MOD", "NOT",
                                "OR",  "SHL", "SHR", "XOR"};

//--------------------------------------------------
// FUNCTIONS
//--------------------------------------------------

const set<string> functions = {
    "ABS",    "ASC",   "ATN",      "ATTR$",        "BASE",      "BIN$",
    "CDBL",   "CHR$",  "CINT",     "COLLISION",    "COS",       "CSNG",
    "CSRLIN", "DATE",  "DBL",      "EOF",          "EXP",       "FIX",
    "FRE",    "HEAP",  "HEX$",     "INKEY",        "INKEY$",    "INP",
    "INPUT$", "INSTR", "INT",      "IPEEK",        "LEFT$",     "LEN",
    "LOG",    "LPOS",  "MAKER",    "MID$",         "MSX",       "NTSC",
    "OCT$",   "PAD",   "PDL",      "PEEK",         "PLYSTATUS", "POINT",
    "POS",    "PSG",   "RESOURCE", "RESOURCESIZE", "RIGHT$",    "RND",
    "SGN",    "SIN",   "SPACE$",   "SPC",          "SPRITE$",   "SNG",
    "SQR",    "STICK", "STR$",     "STRING$",      "STRIG",     "TAB",
    "TAN",    "TILE",  "TIME",     "TURBO",        "USR",       "USR0",
    "USING$", "VAL",   "VARPTR",   "VDP",          "VPEEK"};
}  // namespace

//--------------------------------------------------
// LEXEME CONSTRUCTOR
//--------------------------------------------------

Lexeme::Lexeme() {
  clear();
}

Lexeme::Lexeme(shared_ptr<Lexeme> plexeme) {
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

Lexeme::~Lexeme() = default;

//--------------------------------------------------
// LEXEME METHODS
//--------------------------------------------------

shared_ptr<Lexeme> Lexeme::clone() {
  return make_shared<Lexeme>(*this);
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

const char* Lexeme::getTypeName() {
  return typeNames.at(type);
}

const char* Lexeme::getSubTypeName() {
  return subTypeNames.at(subtype);
}

bool Lexeme::isKeyword() {
  return keywords.count(value) != 0;
}

bool Lexeme::isBooleanOperator() {
  return booleanOps.count(value) != 0;
}

bool Lexeme::isFunction() {
  return functions.count(value) != 0;
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
