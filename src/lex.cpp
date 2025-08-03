//-----------------------------------------------------------
// MSX BASIC lexer classes
// created by Amaury Carvalho, 2019
// reference: https://en.wikipedia.org/wiki/Lexical_analysis
//-----------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include "lex.h"

/***
 * @name Lexeme
 * @class Lexeme
 * @brief Lexeme class constructor. 
 * It represents a simple MSX BASIC lexeme (constant, keyword, identifier etc)
 */
Lexeme::Lexeme() {
    clear();
}

/***
 * @name Lexeme
 * @class Lexeme
 * @brief Lexeme class constructor. 
 * It represents a simple MSX BASIC lexeme (constant, keyword, identifier etc)
 * @parm plexeme Lexeme to clone
 */
Lexeme::Lexeme(Lexeme *plexeme) {
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

/***
 * @name Lexeme
 * @class Lexeme
 * @brief Lexeme class constructor. 
 * It represents a simple MSX BASIC lexeme (constant, keyword, identifier etc)
 * @parm ptype Lexeme type
 * @parm psubtype Lexeme subtype
 * @parm pname Lexeme name
 */
Lexeme::Lexeme(LexemeType ptype, LexemeSubType psubtype, string pname) {
    clear();
    type = ptype;
    subtype = psubtype;
    name = pname;
    value = pname;
}

/***
 * @name Lexeme
 * @class Lexeme
 * @brief Lexeme class constructor. 
 * It represents a simple MSX BASIC lexeme (constant, keyword, identifier etc)
 * @parm ptype Lexeme type
 * @parm psubtype Lexeme subtype
 * @parm pname Lexeme name
 * @parm pvalue Lexeme value
 */
Lexeme::Lexeme(LexemeType ptype, LexemeSubType psubtype, string pname, string pvalue) {
    clear();
    type = ptype;
    subtype = psubtype;
    name = pname;
    value = pvalue;
}

/***
 * @name clone
 * @class Lexeme
 * @brief Clone the current lexeme
 * @return A new lexeme copied from current one 
 */
Lexeme * Lexeme::clone() {
    return new Lexeme(this);
}

/***
 * @name clear
 * @class Lexeme
 * @brief Clear the current lexeme
 */
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

/***
 * @name print
 * @class Lexeme
 * @brief Print to the terminal the current lexeme
 */
void Lexeme::print() {
    if(isArray)
        printf("%*s--> %s %s (array): %s\n", indent, "", getTypeName(), getSubTypeName(), value.c_str());
    else
        printf("%*s--> %s %s: %s\n", indent, "", getTypeName(), getSubTypeName(), value.c_str());
}

/***
 * @name getTypeName
 * @class Lexeme
 * @brief Get the type name of current lexeme
 * @return Type name string
 */
char * Lexeme::getTypeName() {
    static char LexemeTypeName[][20] = { "Unknown", "Identifier", "Keyword", "Separator", "Operator", "Literal", "Comment" };

    return LexemeTypeName[type];
}

/***
 * @name getSubTypeName
 * @class Lexeme
 * @brief Get the subtype name of current lexeme
 * @return Subtype name string
 */
char * Lexeme::getSubTypeName() {
    static char LexemeSubTypeName[][20] = { "", "String", "Numeric", "Single Decimal", "Double Decimal", "Basic String", "Boolean/Remainder", "Function", "Null", "Binary Data", "Integer Data", "Unknown" };

    return LexemeSubTypeName[subtype];
}

/***
 * @name isKeyword
 * @class Lexeme
 * @brief Is the current lexeme a keyword?
 * @param pvalue Keyword to compare against
 * @return True or false
 */
bool Lexeme::isKeyword(string pvalue) {
    return (type == Lexeme::type_keyword && value == pvalue);
}

/***
 * @name isSeparator
 * @class Lexeme
 * @brief Is the current lexeme a separator?
 * @param pvalue Separator to compare against
 * @return True or false
 */
bool Lexeme::isSeparator(string pvalue) {
    return (type == Lexeme::type_separator && value == pvalue);
}

/***
 * @name isOperator
 * @class Lexeme
 * @brief Is the current lexeme an operator?
 * @param pvalue Operator to compare against
 * @return True or false
 */
bool Lexeme::isOperator(string pvalue) {
    return (type == Lexeme::type_operator && value == pvalue);
}

/***
 * @name isLiteralNumeric
 * @class Lexeme
 * @brief Is the current lexeme a literal numeric?
 * @return True or false
 */
bool Lexeme::isLiteralNumeric() {
    return (type == Lexeme::type_literal && subtype == Lexeme::subtype_numeric);
}

/***
 * @name isKeyword
 * @class Lexeme
 * @brief Is the current lexeme a valid keyword?
 * @return True or false
 */
bool Lexeme::isKeyword() {
    // https://www.msx.org/wiki/Category:MSX-BASIC_Instructions
    return ( value == "CLS" || value == "PRINT" || value == "END" || value == "GOTO"
             || value == "IF" || value == "THEN" || value == "ELSE" || value == "COLOR"
             || value == "GOSUB" || value == "FOR" || value == "NEXT" || value == "RETURN"
             || value == "ON" || value == "OFF" || value == "STOP"
             || value == "MOD" || value == "CALL" || value == "PAUSE"
             || value == "INTERVAL" || value == "ERROR" || value == "FONT"
             || value == "GET" || value == "DATE" || value == "TIME" || value == "SET"
             || value == "ASC" || value == "BIN$" || value == "CDBL" || value == "CHR$"
             || value == "CINT" || value == "CSNG" || value == "HEX$" || value == "OCT$"
             || value == "VAL" || value == "WIDTH" || value == "SPC" || value == "STR$"
             || value == "BLOAD" || value == "BSAVE" || value == "CLOAD" || value == "CSAVE"
             || value == "CLOSE" || value == "EOF" || value == "LOAD"
             || value == "MAX" || value == "FILES" || value == "LEN" || value == "USING$"
             || value == "MERGE" || value == "MOTOR" || value == "OPEN" || value == "RUN"
             || value == "SAVE" || value == "VARPTR" || value == "NEW" || value == "USING"
             || value == "BASE" || value == "CIRCLE" || value == "COPY" || value == "SCREEN"
             || value == "CSRLIN" || value == "DRAW" || value == "LINE" || value == "LOCATE"
             || value == "LPOS" || value == "PAINT" || value == "POINT" || value == "POS"
             || value == "PRESET" || value == "PSET" || value == "DRAW" || value == "INP"
             || value == "PEEK" || value == "POKE" || value == "VPEEK" || value == "VPOKE"
             || value == "IPEEK" || value == "IPOKE"
             || value == "BASE" || value == "VDP" || value == "DEFINT" || value == "DEFDBL"
             || value == "DEFSNG" || value == "DEFSTR" || value == "DEF" || value == "FN"
             || value == "OUT" || value == "WAIT" || value == "INPUT$"
             || value == "INPUT" || value == "KEY" || value == "STRIG" || value == "STEP"
             || value == "PAD" || value == "PDL" || value == "STICK" || value == "AND"
             || value == "EQV" || value == "IMP" || value == "NOT" || value == "OR"
             || value == "XOR" || value == "ABS" || value == "CDBL" || value == "CINT"
             || value == "CSNG" || value == "EXP" || value == "FIX" || value == "INT"
             || value == "LOG" || value == "RND" || value == "SGN" || value == "SQR"
             || value == "DEF" || value == "USR" || value == "LLIST" || value == "LPRINT"
             || value == "BEEP" || value == "PLAY" || value == "SOUND" || value == "SPRITE$"
             || value == "SPRITE" || value == "INSTR" || value == "LEFT$" || value == "MID$"
             || value == "RIGHT$" || value == "SPACE$" || value == "STRING$" || value == "ATN"
             || value == "COS" || value == "SIN" || value == "TAN" || value == "CLEAR"
             || value == "DATA" || value == "DIM" || value == "ERASE" || value == "LET"
             || value == "REDIM" || value == "TO" || value == "AND" || value == "TAB"
             || value == "REM" || value == "READ" || value == "RESTORE" || value == "RUN"
             || value == "SWAP" || value == "SEED" || value == "RANDOMIZE" || value == "RESUME"
             || value == "SHR" || value == "SHL" || value == "INKEY$" || value == "INKEY"
             || value == "KANJI" || value == "PUT" || value == "TPSET" || value == "VIDEO"
             || value == "TAND" || value == "TOR" || value == "TPRESET" || value == "TXOR"
             || value == "PAGE" || value == "SCROLL" || value == "FRE" || value == "HEAP"
             || value == "TILE" || value == "TILES" || value == "MSX" || value == "RESOURCE" || value == "RESOURCESIZE"
             || value == "INCLUDE" || value == "TURBO" || value == "TEXT" || value == "CMD" || value == "FILE"
             || value == "RUNASM" || value == "RUNBAS" || value == "WRTVRAM" || value == "WRTCHR"
             || value == "WRTCLR" || value == "WRTSCR" || value == "WRTSPRPAT" || value == "WRTSPRCLR"
             || value == "WRTSPRATR" || value == "RAMTOVRAM" || value == "VRAMTORAM" || value == "RAMTORAM" || value == "RSCTORAM"
             || value == "PT3LOAD" || value == "PT3PLAY" || value == "PT3MUTE" || value == "PT3LOOP" || value == "PT3REPLAY"
             || value == "PLYLOAD" || value == "PLYSONG" || value == "PLYPLAY" || value == "PLYMUTE" || value == "PLYLOOP"
             || value == "PLYREPLAY" || value == "PLYSOUND" || value == "PLYSTATUS"
             || value == "DISSCR" || value == "ENASCR" || value == "WRTFNT" || value == "SETFNT"
             || value == "?" || value == "_" || value == "CLRSCR" || value == "KEYCLKOFF"
             || value == "CLRKEY" || value == "COLLISION" || value == "MUTE"
             || value == "PSG" || value == "NTSC" || value == "IDATA" || value == "IREAD" || value == "IRESTORE"
             || value == "MAKER" || value == "UPDFNTCLR"
             || value == "PATTERN" || value == "FLIP" || value == "ROTATE"
             || value == "FROM" || value == "PASTE" || value == "ADJUST" || value == "TITLE" || value == "PROMPT"
           );
}

/***
 * @name isBooleanOperator
 * @class Lexeme
 * @brief Is the current lexeme a valid boolean operator?
 * @return True or false
 */
bool Lexeme::isBooleanOperator() {
    // https://www.msx.org/wiki/Category:MSX-BASIC_Instructions
    return ( value == "AND" || value == "OR" || value == "XOR" || value == "MOD"
             || value == "IMP" || value == "EQV" || value == "NOT" || value == "SHR" || value == "SHL" );
}

/***
 * @name isFunction
 * @class Lexeme
 * @brief Is the current lexeme a valid function keyword?
 * @return True or false
 */
bool Lexeme::isFunction() {
    // https://www.msx.org/wiki/Category:MSX-BASIC_Instructions
    return ( value == "DATE" || value == "TIME" || value == "ASC" || value == "BIN$"
             || value == "CDBL" || value == "CHR$" || value == "SPC"
             || value == "CINT" || value == "CSNG" || value == "HEX$" || value == "OCT$"
             || value == "VAL" || value == "EOF" || value == "VARPTR" || value == "STR$"
             || value == "CSRLIN" || value == "LPOS" || value == "POINT" || value == "POS"
             || value == "PEEK" || value == "LEN" || value == "FRE" || value == "HEAP"
             || value == "VPEEK" || value == "INP" || value == "BASE" || value == "VARPTR"
             || value == "IPEEK"
             || value == "INKEY" || value == "INPUT$" || value == "INKEY$"
             || value == "STRIG" || value == "PAD" || value == "PDL" || value == "STICK"
             || value == "ABS" || value == "CDBL" || value == "VDP" || value == "SNG"
             || value == "EXP" || value == "FIX" || value == "INT" || value == "DBL"
             || value == "LOG" || value == "RND" || value == "SGN" || value == "SQR"
             || value == "SPRITE$" || value == "INSTR" || value == "LEFT$" || value == "MID$"
             || value == "RIGHT$" || value == "SPACE$" || value == "STRING$" || value == "ATN"
             || value == "COS" || value == "SIN" || value == "TAN" || value == "TAB"
             || value == "ATTR$" || value == "USR0" || value == "USR" || value == "USING$"
             || value == "COLLISION" || value == "TILE" || value == "MSX" || value == "RESOURCE"
             || value == "RESOURCESIZE"
             || value == "PSG" || value == "NTSC" || value == "TURBO" || value == "MAKER"
             || value == "PLYSTATUS" );
}

/***
 * @name LexerLine
 * @class LexerLine
 * @brief LexerLine class constructor. 
 * It represents a set of MSX BASIC lexemes forming a line.
 */
LexerLine::LexerLine() {
    lexemeIndex = 0;
}

/***
 * @name clearLexemes
 * @class LexerLine
 * @brief Clear the current line lexeme list
 */
void LexerLine::clearLexemes() {
    lexemes.clear();
}

/***
 * @name addLexeme
 * @class LexerLine
 * @brief Add a lexeme to the line list
 * @param lexeme Lexeme object
 */
void LexerLine::addLexeme(Lexeme *lexeme) {
    lexemes.push_back(lexeme);
}

/***
 * @name setLexemeBOF
 * @class LexerLine
 * @brief Set index at before the top of the lexeme list
 */
void LexerLine::setLexemeBOF() {
    lexemeIndex = -1;
}

/***
 * @name getCurrentLexeme
 * @class LexerLine
 * @brief Get the current lexeme in the lexeme list
 * @return Lexeme object
 */
Lexeme *LexerLine::getCurrentLexeme() {
    return getLexeme(lexemeIndex);
}

/***
 * @name getFirstLexeme
 * @class LexerLine
 * @brief Get the first lexeme in the lexeme list
 * @return Lexeme object
 */
Lexeme *LexerLine::getFirstLexeme() {
    return getLexeme(0);
}

/***
 * @name getNextLexeme
 * @class LexerLine
 * @brief Get the next lexeme in the lexeme list
 * @return Lexeme object
 */
Lexeme *LexerLine::getNextLexeme() {
    return getLexeme(lexemeIndex + 1);
}

/***
 * @name getPreviousLexeme
 * @class LexerLine
 * @brief Get the previous lexeme in the lexeme list
 * @return Lexeme object
 */
Lexeme *LexerLine::getPreviousLexeme() {
    return getLexeme(lexemeIndex - 1);
}

/***
 * @name getLastLexeme
 * @class LexerLine
 * @brief Get the last lexeme in the lexeme list
 * @return Lexeme object
 */
Lexeme *LexerLine::getLastLexeme() {
    return getLexeme(lexemes.size() - 1);
}

/***
 * @name getLexeme
 * @class LexerLine
 * @brief Get an item in the lexeme list
 * @param i Index
 * @return Lexeme object
 */
Lexeme* LexerLine::getLexeme(int i) {
    if(i >= 0 && i < (int)lexemes.size()) {
        lexemeIndex = i;
        return lexemes[i];
    } else
        return 0;
}

/***
 * @name getLexemeCount
 * @class LexerLine
 * @brief Get the lexeme list items count
 * @return Lexeme count
 */
int LexerLine::getLexemeCount() {
    return lexemes.size();
}

/***
 * @name pushLexeme
 * @class LexerLine
 * @brief Push the current lexeme to the stack
 */
void LexerLine::pushLexeme() {
    lexemeStack.push(lexemeIndex);
}

/***
 * @name popLexeme
 * @class LexerLine
 * @brief Pop a lexeme from the stack, also setting it as current lexeme
 */
void LexerLine::popLexeme() {
    if(!lexemeStack.empty()) {
        lexemeIndex = lexemeStack.top();
        lexemeStack.pop();
    }
}

/***
 * @name popLexemeDiscarding
 * @class LexerLine
 * @brief Discard a lexeme from the stack, but keeping the current lexeme
 */
void LexerLine::popLexemeDiscarding() {
    lexemeStack.pop();
}

/***
 * @name print
 * @class LexerLine
 * @brief Print to the terminal the lexemes from the current line
 */
void LexerLine::print() {
    printf("%s", line.c_str());
    for(unsigned int i=0; i < lexemes.size(); i++)
        lexemes[i]->print();
}

/***
 * @name evaluate
 * @class LexerLine
 * @brief Evaluates the current line by performing a lexical analysis on it
 * @return True, if a valid line
 */
bool LexerLine::evaluate() {
    int i, t;
    char c;
    bool hexa=false;
    Lexeme *lexeme;

    lexemes.clear();

    t = line.length();

    lexeme = new Lexeme();

    for(i = 0; i < t; i++) {

        c = line[i];

        switch(lexeme->type) {
            case Lexeme::type_unknown: {
                    if( c <= ' ' ) {
                        continue;
                    } else if( isDecimal(c) || c == '"' || c == '&' ) {
                        lexeme->type = Lexeme::type_literal;
                        lexeme->value += c;
                        hexa = (c == '&');
                        if(c == '.')
                            lexeme->subtype = Lexeme::subtype_double_decimal;
                        else if (c == '"')
                            lexeme->subtype = Lexeme::subtype_string;
                        else
                            lexeme->subtype = Lexeme::subtype_numeric;
                        continue;
                    } else if( isOperator(c) ) {
                        lexeme->type = Lexeme::type_operator;
                        lexeme->value += c;
                        lexemes.push_back( lexeme );
                        lexeme = new Lexeme();
                        continue;
                    } else if( isSeparator(c) ) {
                        lexeme->type = Lexeme::type_separator;
                        lexeme->value += c;
                        lexemes.push_back( lexeme );
                        lexeme = new Lexeme();
                        continue;
                    } else if( isIdentifier(c, true) ) {
                        lexeme->type = Lexeme::type_identifier;
                        lexeme->subtype = Lexeme::subtype_single_decimal;   // default identifier subtype
                        lexeme->value += toupper(c);
                        lexeme->name = lexeme->value;
                        continue;
                    } else if( c == '?' || c == '_' ) {
                        lexeme->type = Lexeme::type_identifier;
                        lexeme->subtype = Lexeme::subtype_any;
                        lexeme->value += toupper(c);
                        lexeme->name = lexeme->value;
                        continue;
                    } else if( c == '#' ) {
                        lexeme->type = Lexeme::type_separator;
                        lexeme->value += c;
                        lexemes.push_back( lexeme );
                        lexeme = new Lexeme();
                        continue;
                    } else if( c == '\'' ) {
                        lexeme->type = Lexeme::type_operator;
                        lexeme->subtype = Lexeme::subtype_any;
                        lexeme->value = "\'";
                        lexeme->name = lexeme->value;
                        lexemes.push_back( lexeme );
                        if(line[i+1] == '#') {   // if xbasic special commands...
                            char s[255];
                            strcpy(s, &line[i+1]);
                            s[strlen(s)-1] = 0;
                            lexeme = new Lexeme(Lexeme::type_comment, Lexeme::subtype_any, s);
                            lexemes.push_back( lexeme );
                        }
                        return true;
                    } else {
                        lexeme->type = Lexeme::type_unknown;
                        lexeme->value += c;
                        lexemes.push_back( lexeme );
                        return false;
                    }
                }
                break;

            case Lexeme::type_literal: {
                    if( lexeme->subtype == Lexeme::subtype_string ) {
                        lexeme->value += c;
                        if( c == '"' ) {
                            lexemes.push_back( lexeme );
                            lexeme = new Lexeme();
                        }
                        continue;

                    } else {
                        if( hexa ) {
                            if( isDecimal(c) || isHexDecimal(c) ) {
                                lexeme->value += c;
                            } else {
                                lexemes.push_back( lexeme );
                                lexeme = new Lexeme();
                                if( c > ' '  )
                                    i--;
                            }
                        } else if( isDecimal(c) ) {
                            lexeme->value += c;
                            if( lexeme->subtype == Lexeme::subtype_double_decimal && c == '.' ) {    //strchr(lexeme->value.c_str(), '.' ) ) {
                                lexeme->type = Lexeme::type_unknown;
                                lexeme->subtype = Lexeme::subtype_any;
                                lexemes.push_back( lexeme );
                                return false;
                            } else {
                                if( c == '.' ) {
                                    lexeme->subtype = Lexeme::subtype_double_decimal;
                                }
                                if(lexeme->subtype == Lexeme::subtype_numeric) {
                                    if(lexeme->value.size() > 5 || (lexeme->value.size() == 5 && lexeme->value > "32767")) {
                                        lexeme->subtype = Lexeme::subtype_single_decimal;
                                    }
                                }
                                if(lexeme->subtype == Lexeme::subtype_single_decimal) {
                                    if(lexeme->value.size() > 6) {
                                        lexeme->subtype = Lexeme::subtype_double_decimal;
                                    }
                                }
                            }
                        } else if(c == '%') {
                            lexeme->subtype = Lexeme::subtype_numeric;
                        } else if(c == '#') {
                            lexeme->subtype = Lexeme::subtype_single_decimal;
                        } else if(c == '!') {
                            lexeme->subtype = Lexeme::subtype_double_decimal;
                        } else {
                            lexemes.push_back( lexeme );
                            lexeme = new Lexeme();
                            if( c > ' '  )
                                i--;
                        }
                        continue;
                    }

                }
                break;

            case Lexeme::type_identifier: {
                    if( c <= ' ' || ! isIdentifier(c, false) ) {
                        if(lexeme->isKeyword()) {
                            lexeme->type = Lexeme::type_keyword;
                            lexeme->subtype = Lexeme::subtype_any;
                            if(lexeme->isBooleanOperator()) {
                                lexeme->type = Lexeme::type_operator;
                                lexeme->subtype = Lexeme::subtype_boolean_operator;
                            } else if(lexeme->isFunction()) {
                                lexeme->subtype = Lexeme::subtype_function;
                            }
                        }
                        lexemes.push_back( lexeme );
                        lexeme = new Lexeme();
                        if( c > ' ')
                            i --;
                    } else {
                        lexeme->value += toupper(c);
                        lexeme->name = lexeme->value;
                        if(lexeme->isKeyword()) {
                            bool isAnotherKeyword = false;
                            for(int ii = i+1; ii < t && ii < (i+20); ii++) {
                                c = line[ii];
                                lexeme->value += toupper(c);
                                if(lexeme->isKeyword()) {
                                    isAnotherKeyword = true;
                                    break;
                                }
                            }
                            lexeme->value = lexeme->name;
                            if(isAnotherKeyword)
                                continue;
                            lexeme->type = Lexeme::type_keyword;
                            lexeme->subtype = Lexeme::subtype_any;
                            if(lexeme->isBooleanOperator()) {
                                lexeme->type = Lexeme::type_operator;
                                lexeme->subtype = Lexeme::subtype_boolean_operator;
                            } else if(lexeme->isFunction()) {
                                lexeme->subtype = Lexeme::subtype_function;
                            }
                            lexemes.push_back( lexeme );
                            if(lexeme->value == "REM") //|| lexeme->value == "\'")
                                return true;
                            lexeme = new Lexeme();
                        } else {
                            // VALTYP: %=2 $=3 !=4 #=8
                            if     ( c == '%' )
                                lexeme->subtype = Lexeme::subtype_numeric;
                            else if( c == '$' )
                                lexeme->subtype = Lexeme::subtype_string;
                            else if( c == '!' )
                                lexeme->subtype = Lexeme::subtype_single_decimal;
                            else if( c == '#' )
                                lexeme->subtype = Lexeme::subtype_double_decimal;
                        }
                    }
                    continue;
                }
                break;

            case Lexeme::type_keyword: {
                    if( c <= ' ' || ! isIdentifier(c, false) ) {
                        lexemes.push_back( lexeme );
                        lexeme = new Lexeme();
                        if( c > ' ')
                            i --;
                    } else
                        lexeme->value += toupper(c);
                    continue;
                }
                break;

            case Lexeme::type_operator: {
                    if( c <= ' ' || ! isOperator(c) ) {
                        lexemes.push_back( lexeme );
                        lexeme = new Lexeme();
                        if( c > ' ' )
                            i --;
                    } else
                        lexeme->value += c;
                    continue;
                }
                break;

            case Lexeme::type_separator: {
                    if( c <= ' ' || ! isSeparator(c) ) {
                        lexemes.push_back( lexeme );
                        lexeme = new Lexeme();
                        if( c > ' ' )
                            i --;
                    } else
                        lexeme->value += c;
                    continue;
                }
                break;

            case Lexeme::type_comment: {
                    if( c <= ' ' || ! isComment(c) ) {
                        lexemes.push_back( lexeme );
                        lexeme = new Lexeme();
                        if( c > ' ' )
                            i --;
                    } else
                        lexeme->value += c;
                    continue;
                }
                break;

        }


    }

    if( lexeme->type != Lexeme::type_unknown ) {
        if(lexeme->isKeyword()) {
            lexeme->type = Lexeme::type_keyword;
            lexeme->subtype = Lexeme::subtype_any;
            if(lexeme->isBooleanOperator()) {
                lexeme->type = Lexeme::type_operator;
                lexeme->subtype = Lexeme::subtype_boolean_operator;
            } else if(lexeme->isFunction()) {
                lexeme->subtype = Lexeme::subtype_function;
            }
        }
        lexemes.push_back( lexeme );
    } else
        free(lexeme);

    return true;
}

/***
 * @name isNumeric
 * @class LexerLine
 * @brief Check if the character is numeric
 * @param c Character
 * @return True or false
 */
bool LexerLine::isNumeric(char c) {
    return (c >= '0' && c <= '9');
}

/***
 * @name isDecimal
 * @class LexerLine
 * @brief Check if the character is a decimal point
 * @param c Character
 * @return True or false
 */
bool LexerLine::isDecimal(char c) {
    return isNumeric(c) || c == '.';
}

/***
 * @name isHexDecimal
 * @class LexerLine
 * @brief Check if the character is hexadecimal
 * @param c Character
 * @return True or false
 */
bool LexerLine::isHexDecimal(char c) {
    return isNumeric(c) || c == 'B' || c == 'b' || c == 'h' || c == 'H' || c == 'o' || c == 'O' ||
           (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

/***
 * @name isSeperator
 * @class LexerLine
 * @brief Check if the character is a separator
 * @param c Character
 * @return True or false
 */
bool LexerLine::isSeparator(char c) {
    return (c == ':' || c == '(' || c == ')' || c == '{' || c == '}' || c == ',' || c == ';');
}

/***
 * @name isOperator
 * @class LexerLine
 * @brief Check if the character is an operator
 * @param c Character
 * @return True or false
 */
bool LexerLine::isOperator(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '<' || c == '>' || c == '^' || c == '\\' ); //|| c == '\''); remark quote symbol
}

/***
 * @name isIdentifier
 * @class LexerLine
 * @brief Check if the character is an identifier
 * @param c Character
 * @param start Is it the first character from the string?
 * @return True or false
 */
bool LexerLine::isIdentifier(char c, bool start) {
    return ( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' ||
             ( ( c == '%' || c == '$' || c == '!' || c =='#' || (c >= '0' && c <= '9') ) && !start) );
}

/***
 * @name isComment
 * @class LexerLine
 * @brief Check if the character is in a commentary string
 * @param c Character
 * @return True or false
 */
bool LexerLine::isComment(char c) {
    return false;
}

/***
 * @name Lexer
 * @class Lexer
 * @brief Lexer class constructor, specialized as a MSX BASIC lexical analyzer
 */
Lexer::Lexer() {
    clear();
}

/***
 * @name clear
 * @class Lexer
 * @brief Clear the lines list
 */
void Lexer::clear() {
    errorMessage = "";
    lines.clear();
}

/***
 * @name load
 * @class Lexer
 * @brief Load a MSX BASIC source code (plain text) in the lines list
 * @param filename Source code file name
 * @return True, if a valid MSX BASIC source code
 */
bool Lexer::load(char *filename) {
    FILE *file;
    char line[255];
    unsigned char header[3];
    int len = 255, bytes;
    LexerLine *lexerLine;

    clear();

    if ((file = fopen(filename, "rb"))) {
        memset(header, 0, 3);
        bytes = fread(header, 1, 3, file);
        fclose(file);

        if(bytes==0) {
            errorMessage = "Empty file";
            return false;
        }
        if(header[0] < 0x20 || header[0] > 126) {
            if(header[0] == 0xFF && header[2] == 0x80) {
                sprintf(line, "Tokenized MSX BASIC source code file detected\nSave it as a plain text to use it with MSXBAS2ROM:\nSAVE \"%s\",A", filename);
                errorMessage = line;
                return false;
            } else if(header[0] != 0x0D && header[0] != 0x0A && header[0] != 0x0C) {     // CR LF FF
                errorMessage = "This is not a MSX BASIC source code file.";
                return false;
            }
        }
    } else {
        errorMessage = "File doesn't exist";
        return false;
    }

    if ((file = fopen(filename, "r"))) {
        while ( fgets(line, len, file) ) {
            lexerLine = new LexerLine();
            lexerLine->line = line;
            lines.push_back( lexerLine );
        }

        fclose(file);
    } else {
        errorMessage = "File doesn't exist";
        return false;
    }

    return true;
}

/***
 * @name evaluate
 * @class Lexer
 * @brief Perform a lexical analysis on the lines list
 * @return True, if lexical analysis success
 */
bool Lexer::evaluate() {
    LexerLine *lexerLine;
    for(unsigned int i=0; i < lines.size(); i++) {
        lexerLine = lines[i];
        if(lexerLine)
            if(!lexerLine->evaluate()) {
                lineNo = i + 1;
                errorMessage = lexerLine->line;
                return false;
            }
    }
    return true;
}

/***
 * @name print
 * @class Lexer
 * @brief Print the lines list
 */
void Lexer::print() {
    LexerLine *lexerLine;
    for(unsigned int i=0; i < lines.size(); i++) {
        lexerLine = lines[i];
        if(lexerLine)
            lexerLine->print();
    }
}

/***
 * @name error
 * @class Lexer
 * @brief Print the invalid line
 */
void Lexer::error() {
    LexerLine *lexerLine = lines[lineNo - 1];
    if(lexerLine)
        lexerLine->print();
}

