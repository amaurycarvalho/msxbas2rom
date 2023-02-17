//-----------------------------------------------------------
// Lexer class specialized as a MSX BASIC tokenizer
// created by Amaury Carvalho, 2019
// reference: https://en.wikipedia.org/wiki/Lexical_analysis
//-----------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include "lex.h"

Lexeme::Lexeme() {
    clear();
}

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

Lexeme::Lexeme(LexemeType ptype, LexemeSubType psubtype, string pname) {
    clear();
    type = ptype;
    subtype = psubtype;
    name = pname;
    value = pname;
}

Lexeme::Lexeme(LexemeType ptype, LexemeSubType psubtype, string pname, string pvalue) {
    clear();
    type = ptype;
    subtype = psubtype;
    name = pname;
    value = pvalue;
}

Lexeme * Lexeme::clone() {
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

void Lexeme::print() {
    if(isArray)
        printf("%*s--> %s %s (array): %s\n", indent, "", getTypeName(), getSubTypeName(), value.c_str());
    else
        printf("%*s--> %s %s: %s\n", indent, "", getTypeName(), getSubTypeName(), value.c_str());
}

char * Lexeme::getTypeName() {
    static char LexemeTypeName[][20] = { "Unknown", "Identifier", "Keyword", "Separator", "Operator", "Literal", "Comment" };

    return LexemeTypeName[type];
}

char * Lexeme::getSubTypeName() {
    static char LexemeSubTypeName[][20] = { "", "String", "Numeric", "Single Decimal", "Double Decimal", "Basic String", "Boolean/Remainder", "Function", "Null", "Binary Data", "Integer Data", "Unknown" };

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
             || value == "PRESET" || value == "PSET" || value == "DRAW" || value == "PEEK"
             || value == "POKE" || value == "VPEEK" || value == "VPOKE" || value == "INP"
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
             || value == "WRTSPRATR" || value == "RAMTOVRAM" || value == "VRAMTORAM" || value == "RAMTORAM"
             || value == "PT3LOAD" || value == "PT3PLAY" || value == "PT3MUTE" || value == "PT3LOOP" || value == "PT3REPLAY"
             || value == "PLYLOAD" || value == "PLYSONG" || value == "PLYPLAY" || value == "PLYMUTE" || value == "PLYLOOP"
             || value == "PLYREPLAY" || value == "PLYSOUND" || value == "PLYSTATUS"
             || value == "DISSCR" || value == "ENASCR" || value == "WRTFNT" || value == "SETFNT"
             || value == "?" || value == "_" || value == "CLRSCR" || value == "KEYCLKOFF"
             || value == "CLRKEY" || value == "COLLISION" || value == "MUTE"
             || value == "PSG" || value == "NTSC" || value == "IDATA" || value == "IREAD" || value == "IRESTORE"
             || value == "MAKER" || value == "UPDFNTCLR" || value == "PATTERN" || value == "TRANSPOSE"
             || value == "FROM" || value == "PASTE" || value == "ADJUST" || value == "TITLE" || value == "PROMPT"
           );
}

bool Lexeme::isBooleanOperator() {
    // https://www.msx.org/wiki/Category:MSX-BASIC_Instructions
    return ( value == "AND" || value == "OR" || value == "XOR" || value == "MOD"
             || value == "IMP" || value == "EQV" || value == "NOT" || value == "SHR" || value == "SHL" );
}

bool Lexeme::isFunction() {
    // https://www.msx.org/wiki/Category:MSX-BASIC_Instructions
    return ( value == "DATE" || value == "TIME" || value == "ASC" || value == "BIN$"
             || value == "CDBL" || value == "CHR$" || value == "SPC"
             || value == "CINT" || value == "CSNG" || value == "HEX$" || value == "OCT$"
             || value == "VAL" || value == "EOF" || value == "VARPTR" || value == "STR$"
             || value == "CSRLIN" || value == "LPOS" || value == "POINT" || value == "POS"
             || value == "PEEK" || value == "LEN" || value == "FRE" || value == "HEAP"
             || value == "VPEEK" || value == "INP" || value == "BASE" || value == "VARPTR"
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

LexerLine::LexerLine() {
    lexemeIndex = 0;
}

void LexerLine::clearLexemes() {
    lexemes.clear();
}

void LexerLine::addLexeme(Lexeme *lexeme) {
    lexemes.push_back(lexeme);
}

void LexerLine::setLexemeBOF() {
    lexemeIndex = -1;
}

Lexeme *LexerLine::getCurrentLexeme() {
    return getLexeme(lexemeIndex);
}

Lexeme *LexerLine::getFirstLexeme() {
    return getLexeme(0);
}

Lexeme *LexerLine::getNextLexeme() {
    return getLexeme(lexemeIndex + 1);
}

Lexeme *LexerLine::getPreviousLexeme() {
    return getLexeme(lexemeIndex - 1);
}

Lexeme *LexerLine::getLastLexeme() {
    return getLexeme(lexemes.size() - 1);
}

Lexeme* LexerLine::getLexeme(int i) {
    if(i >= 0 && i < (int)lexemes.size()) {
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
    if(!lexemeStack.empty()) {
        lexemeIndex = lexemeStack.top();
        lexemeStack.pop();
    }
}

void LexerLine::popLexemeDiscarting() {
    lexemeStack.pop();
}

void LexerLine::print() {
    printf("%s", line.c_str());
    for(unsigned int i=0; i < lexemes.size(); i++)
        lexemes[i]->print();
}

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

bool LexerLine::isNumeric(char c) {
    return (c >= '0' && c <= '9');
}

bool LexerLine::isDecimal(char c) {
    return isNumeric(c) || c == '.';
}

bool LexerLine::isHexDecimal(char c) {
    return isNumeric(c) || c == 'B' || c == 'b' || c == 'h' || c == 'H' || c == 'o' || c == 'O' ||
           (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool LexerLine::isSeparator(char c) {
    return (c == ':' || c == '(' || c == ')' || c == '{' || c == '}' || c == ',' || c == ';');
}

bool LexerLine::isOperator(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '<' || c == '>' || c == '^' || c == '\\' ); //|| c == '\''); remark quote symbol
}

bool LexerLine::isIdentifier(char c, bool start) {
    return ( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' ||
             ( ( c == '%' || c == '$' || c == '!' || c =='#' || (c >= '0' && c <= '9') ) && !start) );
}

bool LexerLine::isComment(char c) {
    return false;
}

bool Lexer::load(char *filename) {
    FILE *file;
    char line[255];
    unsigned char header[3];
    int len = 255, bytes;
    LexerLine *lexerLine;

    errorMessage = "";

    lines.clear();

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

void Lexer::print() {
    LexerLine *lexerLine;
    for(unsigned int i=0; i < lines.size(); i++) {
        lexerLine = lines[i];
        if(lexerLine)
            lexerLine->print();
    }
}

void Lexer::error() {
    LexerLine *lexerLine = lines[lineNo - 1];
    if(lexerLine)
        lexerLine->print();
}

