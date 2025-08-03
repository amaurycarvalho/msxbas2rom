#ifndef COMPILER_H
#define COMPILER_H

#include <string.h>
#include <queue>

#include "parse.h"
#include "pletter.h"

#ifdef Win
  #define strcasecmp _strcasecmp
  int _strcasecmp(const char * str1, const char * str2);
#endif

#define COMPILE_MAX_PAGES (16 * 4)
#define COMPILE_CODE_SIZE (COMPILE_MAX_PAGES * 0x4000)
#define COMPILE_RAM_SIZE  (0xFFFF)

extern unsigned char bin_header_bin[];

class SymbolNode {
  public:
    Lexeme *lexeme;
    TagNode *tag;
    int address;
};

class FixNode {
  public:
    SymbolNode *symbol;
    int address;
    int step;
};

class ForNextNode {
  public:
    int index;
    TagNode *tag;
    Lexeme *for_var, *for_to, *for_step;
    FixNode *for_end_mark;
    SymbolNode *for_step_mark;
    ActionNode *for_to_action, *for_step_action;
};

class CodeNode {
  public:
    string name;
    int start;
    int length;
    bool is_code;
    bool debug;
};

class FileNode {
  private:
    unsigned char *s;
    FILE *handle;
    Pletter pletter;
    bool first;
    int bytes;
    unsigned char buf_plain[255], buf_packed[1024];

  public:
    string name;
    int length;
    bool packed;
    int packed_length;
    int blocks;
    Lexeme *current_lexeme;
    Lexeme *first_lexeme;
    unsigned char *buffer;

    unsigned char file_header[255];

    ~FileNode();

    bool create();
    bool open();
    bool eof();
    int read(unsigned char *data, int max_length);
    void write(unsigned char *data, int data_length);
    void close();
    void clear();

    int read();
    int readAsLexeme();
    int readAsLexeme(unsigned char *data, int data_length);
    void stripQuotes(string text, char *buf);
    void getFileExt(char *filename, char *buf);
    string getFileExt();

    bool writeToFile(char *filename, unsigned char *data, int data_length);
    int readFromFile(char *filename, unsigned char *data, int maxlen);
    int ParseTinySpriteFile(char *filename, unsigned char *data, int maxlen);

    void fixAKM(unsigned char *data, int address, int length);
    void fixAKX(unsigned char *data, int address, int length);

};

class Compiler {
    bool evaluate(TagNode *tag);

    void addByte(unsigned char byte);
    void addByteOptimized(unsigned char byte);
    void addWord(unsigned int word);
    void addWord(unsigned char byte1, unsigned char byte2);
    void addCmd(unsigned char byte, unsigned int word);
    void addCodeByte(unsigned char byte);
    void pushLastCode();
    void popLastCode();

    void addNop();
    void addExAF();
    void addExDEHL();
    void addExiSPHL();
    void addExx();
    void addEI();
    void addDI();
    void addPushAF();
    void addPushHL();
    void addPushBC();
    void addPushDE();
    void addPopAF();
    void addPopHL();
    void addPopBC();
    void addPopDE();
    void addPopIX();
    void addPopIY();
    void addXorA();
    void addXorH();
    void addXorE();
    void addXorD();
    void addAndA();
    void addAndD();
    void addAnd(unsigned char n);
    void addOr(unsigned char n);
    void addOrH();
    void addOrL();
    void addOrB();
    void addOrC();
    void addOrE();
    void addCpL();
    void addCpB();
    void addCp(unsigned char n);
    void addIncA();
    void addIncH();
    void addIncL();
    void addIncD();
    void addIncHL();
    void addIncDE();
    void addDecHL();
    void addAdd(unsigned char n);
    void addAddH();
    void addAddL();
    void addAddA();
    void addAddHLHL();
    void addAddHLBC();
    void addAddHLDE();
    void addAdcA(unsigned char n);
    void addSub(unsigned char n);
    void addSbcHLDE();
    void addSbcHLBC();
    void addRRCA();
    void addRLA();
    void addSRAH();
    void addSRLH();
    void addRRL();
    void addRRA();
    void addCPL();
    void addLDI();
    void addLDIR();
    void addDecA();
    void addDecE();
    void addRet();
    void addRetC();
    void addLdA(unsigned char n);
    void addLdAB();
    void addLdAC();
    void addLdAH();
    void addLdAL();
    void addLdAE();
    void addLdAD();
    void addLdAiHL();
    void addLdAiDE();
    void addLdHA();
    void addLdLA();
    void addLdLC();
    void addLdLH();
    void addLdL(unsigned char n);
    void addLdHE();
    void addLdHC();
    void addLdHL();
    void addLdH(unsigned char n);
    void addLdLiHL();
    void addLdBA();
    void addLdBH();
    void addLdBL();
    void addLdBE();
    void addLdBiHL();
    void addLdB(unsigned char n);
    void addLdC(unsigned char n);
    void addLdEiHL();
    void addLdEA();
    void addLdEL();
    void addLdCA();
    void addLdCB();
    void addLdCE();
    void addLdCD();
    void addLdCH();
    void addLdCL();
    void addLdCiHL();
    void addLdDA();
    void addLdDL();
    void addLdDH();
    void addLdDiHL();
    void addLdHB();
    void addLdHL(int n);
    void addLdHLmegarom();
    void addLdBC(int n);
    void addLdDE();
    void addLdDE(int n);
    void addLdIX(int n);
    void addLdAii(int i);
    void addLdHLii(int i);
    void addLdBCii(int i);
    void addLdDEii(int i);
    void addLdIXii(int i);
    void addLdIYii(int i);
    void addLdiiA(int i);
    void addLdiiHL(int i);
    void addLdiiDE(int i);
    void addLdiiSP(int i);
    void addLdSPii(int i);
    void addLdSPHL();
    void addLdiHL(unsigned char n);
    void addLdiHLA();
    void addLdiHLB();
    void addLdiHLC();
    void addLdiHLD();
    void addLdiHLE();
    void addLdiDEA();
    void addJr(unsigned char n);
    void addJrZ(unsigned char n);
    void addJrNZ(unsigned char n);
    void addJrC(unsigned char n);
    void addJrNC(unsigned char n);
    void addJp(int n);
    void addJpZ(int n);
    void addJpNZ(int n);
    void addCall(unsigned int word);
    void addKernelCall(unsigned int word);
    int getKernelCallAddr(unsigned int word);

    bool evalAction(ActionNode *action);
    bool evalActions(ActionNode *action);
    int evalExpression(ActionNode *action);
    int evalOperator(ActionNode *action);
    int evalFunction(ActionNode *action);
    bool evalOperatorParms(ActionNode *action, int parmCount);
    int evalOperatorCast(ActionNode *action);

    bool addVarAddress(ActionNode *action);
    void addTempStr(bool atHL);
    void addCast(int from, int to);
    bool addAssignment(ActionNode *action);

    SymbolNode* getSymbol(Lexeme *lexeme);
    SymbolNode* addSymbol(Lexeme *lexeme);
    SymbolNode* getSymbol(TagNode *tag);
    SymbolNode* addSymbol(TagNode *tag);
    SymbolNode* addSymbol(string line);

    FixNode* addFix(Lexeme *lexeme);
    FixNode* addFix(SymbolNode *symbol);
    FixNode* addFix(string line);
    SymbolNode* addPreMark();
    FixNode* addMark();

    void func_symbols();
    void clear_symbols();
    void data_symbols();
    int save_symbols();
    void do_fix();

    void double2FloatLib(double value, int *words);
    void float2FloatLib(float value, int *words);
    int str2FloatLib(string value);
    int getUsingFormat(string text);

    void cmd_start();
    void cmd_end(bool last);
    void cmd_cls();
    void cmd_print();
    void cmd_input(bool question);
    void cmd_beep();
    void cmd_goto();
    void cmd_gosub();
    void cmd_return();
    void cmd_sound();
    void cmd_play();
    void cmd_draw();
    void cmd_let();
    void cmd_dim();
    void cmd_redim();
    void cmd_randomize();
    void cmd_if();
    void cmd_for();
    void cmd_next();
    void cmd_locate();
    void cmd_screen();
    void cmd_screen_copy();
    void cmd_screen_paste();
    void cmd_screen_scroll();
    void cmd_screen_load();
    void cmd_screen_on();
    void cmd_screen_off();
    void cmd_color();
    void cmd_width();
    void cmd_pset(bool forecolor);
    void cmd_line();
    void cmd_paint();
    void cmd_circle();
    void cmd_copy();
    void cmd_copy_screen();
    void cmd_data();
    void cmd_idata();
    void cmd_read();
    void cmd_iread();
    void cmd_restore();
    void cmd_irestore();
    void cmd_resume();
    void cmd_out();
    void cmd_poke();
    void cmd_ipoke();
    void cmd_vpoke();
    void cmd_put();
    void cmd_put_sprite();
    void cmd_put_tile();
    void cmd_set();
    void cmd_set_adjust();
    void cmd_set_page();
    void cmd_set_scroll();
    void cmd_set_video();
    void cmd_set_screen();
    void cmd_set_beep();
    void cmd_set_title();
    void cmd_set_prompt();
    void cmd_set_tile();
    void cmd_set_sprite();
    void cmd_set_font();
    void cmd_set_date();
    void cmd_set_time();
    void cmd_get();
    void cmd_get_date();
    void cmd_get_time();
    void cmd_on();
    void cmd_on_error();
    void cmd_on_interval();
    void cmd_on_key();
    void cmd_on_sprite();
    void cmd_on_stop();
    void cmd_on_strig();
    void cmd_on_goto_gosub();
    void cmd_interval();
    void cmd_stop();
    void cmd_sprite();
    void cmd_sprite_load();
    void cmd_key();
    void cmd_strig();
    void cmd_swap();
    void cmd_wait();
    void cmd_file();
    void cmd_text();
    void cmd_call();
    void cmd_cmd();
    void cmd_maxfiles();
    void cmd_open();
    void cmd_close();
    void cmd_def();
    void cmd_bload();

    bool addCheckTraps();
    void addEnableBasicSlot();
    void addDisableBasicSlot();

    void beginBasicSetStmt(string name);
    void endBasicSetStmt();
    void addBasicChar(char c);

    void syntax_error();
    void syntax_error(string msg);

  public:
    Compiler();
    virtual ~Compiler();

    bool build(Parser *parser);
    int write(unsigned char *dest, int start_address);

    float ramMemoryPerc;
    bool pt3, akm, font, file_support, has_defusr;
    bool has_open_grp;
    bool has_tiny_sprite;
    bool megaROM, debug, has_line_number, konamiSCC;

    vector<Lexeme*> resourceList;
    vector<FileNode*> fileList;
    vector<CodeNode*> codeList;
    vector<CodeNode*> dataList;

    int code_start, ram_start, ram_page;
    int code_size, ram_size;
    int segm_last, segm_total;
    string error_message;

    TagNode *current_tag;
    Parser *parser;

    bool compiled;

  protected:
    unsigned char *code; //[20*0xFFFF];
    unsigned char *ram;  //[0xFFFF];

    int code_pointer, ram_pointer;
    int mark_count, for_count;
    unsigned char *last_code[5];

    SymbolNode *heap_mark, *temp_str_mark;
    FixNode *end_mark;
    FixNode *enable_basic_mark, *disable_basic_mark;
    FixNode *draw_mark;
    FixNode *io_redirect_mark, *io_screen_mark;

    ActionNode *current_action;

    vector<SymbolNode*> symbols;
    vector<FixNode*> fixes;
    stack<ForNextNode*> forNextStack;

};

#endif // COMPILER_H
