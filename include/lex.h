#ifndef LEX_H_INCLUDED
#define LEX_H_INCLUDED

#include <string>
#include <vector>
#include <stack>

using namespace std;

class Lexeme {
  public:
    enum LexemeType {
        type_unknown = 0,
        type_identifier, // x, y, up
        type_keyword,    // if, print, color
        type_separator,  // { } ( ) , : ;
        type_operator,   // + - * / < > =
        type_literal,    // true, 6, 1.0, "text"
        type_comment     // text
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

    void clear();
    Lexeme *clone();
    void print();
    char * getTypeName();
    char * getSubTypeName();

    bool isKeyword();
    bool isKeyword(string pvalue);
    bool isSeparator(string pvalue);
    bool isLiteralNumeric();
	bool isBooleanOperator();
	bool isFunction();

	static Lexeme* factory(LexemeType ptype, LexemeSubType psubtype, string pname);
	static Lexeme* factory(LexemeType ptype, LexemeSubType psubtype, string pname, string pvalue);
	static Lexeme* factory(Lexeme *plexeme);

    Lexeme();
	Lexeme(Lexeme *plexeme);
	Lexeme(LexemeType ptype, LexemeSubType psubtype, string pname);
	Lexeme(LexemeType ptype, LexemeSubType psubtype, string pname, string pvalue);

};

class LexerLine {
  private:
    int lexemeIndex;
    vector<Lexeme*> lexemes;
    stack<int> lexemeStack;

	bool isNumeric(char c);
	bool isDecimal(char c);
	bool isHexDecimal(char c);
	bool isSeparator(char c);
	bool isOperator(char c);
	bool isIdentifier(char c, bool start);
	bool isComment(char c);

  public:
    string line;

    bool evaluate();
    void print();

    void clearLexemes();
    void addLexeme(Lexeme *lexeme);

    Lexeme *getFirstLexeme();
    Lexeme *getCurrentLexeme();
    Lexeme *getNextLexeme();
    Lexeme *getPreviousLexeme();
    Lexeme *getLastLexeme();
    Lexeme *getLexeme(int i);
    void setLexemeBOF();
    int getLexemeCount();
    void pushLexeme();
    void popLexeme();
    void popLexemeDiscarting();

    LexerLine();
};

class Lexer {
  public:
    int lineNo=0;
    vector<LexerLine*> lines;
    string errorMessage;

    bool load(char *filename);
    bool evaluate();

    void print();
    void error();

};

#endif // LEX_H_INCLUDED
