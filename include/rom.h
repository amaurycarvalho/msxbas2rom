#ifndef ROM_H_INCLUDED
#define ROM_H_INCLUDED

#include <string.h>
#include <string>
#include <vector>
#include <math.h>

#include "lex.h"
#include "tokenizer.h"
#include "compiler.h"
#include "compiler_pt3.h"

#define ROM_DATA_SIZE (20 * 0x4000)

using namespace std;

class Rom {
  private:
    Tokenizer *tokenizer;
    Compiler *compiler;

    string errorMessage;
    bool errorFound;
    int xbcAddr, pt3Addr;
    int hdrAddr, rtnAddr, mapAddr, txtAddr, filAddr, basAddr, rscAddr;
    int mapInd, txtInd, filInd, basInd;
    int hdrLen, rtnLen, mapLen, txtLen, filLen, pt3Len, basLen;
    int rscSgm, rscLen;

    unsigned char *data;  //[20 * 0x4000];
    bool writePage[20];

    vector<unsigned char*> lines;

    //void buildInit(vector<Lexeme*> *resourceList);
    void buildInit();
    void buildHeader();
    void buildRoutines();
    void buildCompilerRoutines();
    void buildCompilerRoutinesPT3();
    void buildMap(vector<Lexeme*> *resourceList, bool font);
    void buildResources(vector<Lexeme*> *resourceList);
    void buildMapAndResources();
    void buildMapAndResourcesText(Lexeme *lexeme);
    void buildMapAndResourcesData(Parser *parser);
    void buildMapAndResourcesFile(Lexeme *lexeme);
    void buildMapAndResourcesFileTXT(char *filename);
    void buildMapAndResourcesFileCSV(char *filename);
    void buildMapAndResourcesFileSPR(char *filename);
    void buildMapAndResourcesFileSCR(char *filename);
    void buildMapAndResourcesFileBIN(char *filename, char *fileext);
    void addResourceToMap(int offset, int length);
    void buildXBASIC();
    void buildPT3TOOLS();
    void buildFontResources();

    void buildBasicCode();
    void buildCompiledCode();

    void calcBasicLineAddress(TokenLine *line);
    int getBasicLineAddress(int number);
    void buildBasicLine(TokenLine *line);
    void buildTurboLine();
    void buildFilesLine();
    void buildAssemblyLine();
    void buildHeaderAdjust();

    void writeRom(char *filename);

  public:
    int code_start;
    double stdMemoryPerc, rscMemoryPerc;
    bool turbo, xtd, stripRemLines;

    FileNode file;

    Rom();
    ~Rom();

    bool build(Tokenizer *tokenizer, char *filename);
    bool build(Compiler *compiler, char *filename);
    bool build(CompilerPT3 *compiler, char *filename);

    void error();

};


#endif // ASSEMBLY_H_INCLUDED
