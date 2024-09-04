#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED

#include <string>
#include <vector>
#include <stack>
#include "lex.h"

using namespace std;

class ActionNode {
  private:
    int indent;
    void create(Lexeme *plexeme);
  public:
    int subtype;
    Lexeme* lexeme;
    vector<ActionNode*> actions;
    void print();
    ActionNode();
    ActionNode(Lexeme *plexeme);
    ActionNode(string name);
};

class TagNode {
  public:
    string name;
    string value;
    vector<ActionNode*> actions;
    void print();
};

class Parser {
  private:
    bool eval_line(LexerLine *lexerLine);
    bool eval_phrase(LexerLine *phrase);
    bool eval_statement(LexerLine *statement);
    bool eval_assignment(LexerLine *assignment);
    bool eval_expression(LexerLine *parm);
    bool eval_expression_push(LexerLine *parm);
    void eval_expression_pop(int n);

    bool eval_cmd_generic(LexerLine *statement);
    bool eval_cmd_let(LexerLine *statement);
    bool eval_cmd_dim(LexerLine *statement);
    bool eval_cmd_def(LexerLine *statement, int vartype);
    bool eval_cmd_def_usr(LexerLine *statement);
    bool eval_cmd_print(LexerLine *statement);
    bool eval_cmd_input(LexerLine *statement);
    bool eval_cmd_line_input(LexerLine *statement);
    bool eval_cmd_put(LexerLine *statement);
    bool eval_cmd_put_sprite(LexerLine *statement);
    bool eval_cmd_put_tile(LexerLine *statement);
    bool eval_cmd_base(LexerLine *statement);
    bool eval_cmd_vdp(LexerLine *statement);
    bool eval_cmd_time(LexerLine *statement);
    bool eval_cmd_color(LexerLine *statement);
    bool eval_cmd_color_rgb(LexerLine *statement);
    bool eval_cmd_color_sprite(LexerLine *statement);
    bool eval_cmd_if(LexerLine *statement, int level);
    bool eval_cmd_for(LexerLine *statement);
    bool eval_cmd_next(LexerLine *statement);
    bool eval_cmd_pset(LexerLine *statement);
    bool eval_cmd_line(LexerLine *statement);
    bool eval_cmd_circle(LexerLine *statement);
    bool eval_cmd_paint(LexerLine *statement);
    bool eval_cmd_copy(LexerLine *statement);
    bool eval_cmd_screen(LexerLine *statement);
    bool eval_cmd_screen_copy(LexerLine *statement);
    bool eval_cmd_screen_paste(LexerLine *statement);
    bool eval_cmd_screen_scroll(LexerLine *statement);
    bool eval_cmd_screen_load(LexerLine *statement);
    bool eval_cmd_set(LexerLine *statement);
    bool eval_cmd_set_adjust(LexerLine *statement);
    bool eval_cmd_set_tile(LexerLine *statement);
    bool eval_cmd_set_tile_colpat(LexerLine *statement);
    bool eval_cmd_set_sprite(LexerLine *statement);
    bool eval_cmd_set_sprite_colpattra(LexerLine *statement);
    bool eval_cmd_get(LexerLine *statement);
    bool eval_cmd_on(LexerLine *statement);
    bool eval_cmd_on_goto_gosub(LexerLine *statement);
    bool eval_cmd_on_error(LexerLine *statement);
    bool eval_cmd_on_interval(LexerLine *statement);
    bool eval_cmd_on_key(LexerLine *statement);
    bool eval_cmd_on_sprite(LexerLine *statement);
    bool eval_cmd_on_stop(LexerLine *statement);
    bool eval_cmd_on_strig(LexerLine *statement);
    bool eval_cmd_interval(LexerLine *statement);
    bool eval_cmd_stop(LexerLine *statement);
    bool eval_cmd_key(LexerLine *statement);
    bool eval_cmd_strig(LexerLine *statement);
    bool eval_cmd_sprite(LexerLine *statement);
    bool eval_cmd_sprite_load(LexerLine *statement);
    bool eval_cmd_data(LexerLine *statement, Lexeme::LexemeSubType subtype);
    bool eval_cmd_call(LexerLine *statement);
    bool eval_cmd_cmd(LexerLine *statement);
    bool eval_cmd_open(LexerLine *statement);
    bool eval_cmd_close(LexerLine *statement);
    bool eval_cmd_maxfiles(LexerLine *statement);

    bool loadInclude(Lexeme *lexeme);
    int gfxOperatorCode(Lexeme *lexeme);

    int getOperatorPrecedence(Lexeme *lexeme);
    int getOperatorParmCount(Lexeme *lexeme);
    ActionNode * pushActionFromLexeme(Lexeme *lexeme);
    void pushStackFromLexeme(Lexeme *lexeme);

    void pushActionRoot(ActionNode *action);
    void popActionRoot();

    Lexeme * coalesceSymbols(Lexeme *lexeme);

    TagNode *tag;
    ActionNode *actionRoot;
    LexerLine *error_line;
    Lexeme *lex_null, *lex_index;

    stack<ActionNode*> actionStack;
    stack<Lexeme*> expressionList;

    int deftbl[26];            // DEFTBL

    bool eval_expr_error, line_comment;
    string error_message;

  public:
    int lineNo;
    vector<TagNode*> tags;      // abstract syntax tree in list form implementation
    vector<Lexeme*> symbolList; // symbols list coalesced
    vector<Lexeme*> datas;

    bool debug;
    bool has_traps, has_defusr, has_data;
    bool has_play, has_input, has_font;
    bool has_pt3, has_akm, has_resource_restore;
    int resourceCount;

    bool evaluate(Lexer *lexer);

    void print();
    void error();

    Parser();

};

#endif // PARSE_H_INCLUDED
