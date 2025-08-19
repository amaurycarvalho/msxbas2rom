/***
 * @file rom.h
 * @brief ROM builder class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 *   https://www.msx.org/wiki/Develop_a_program_in_cartridge_ROM
 *   https://www.msx.org/wiki/MegaROM_Mappers
 *   https://aoineko.org/msxgl/index.php?title=Create_a_plain_ROM
 *   https://aoineko.org/msxgl/index.php?title=Create_a_mapped_ROM
 */

#ifndef ROM_H_INCLUDED
#define ROM_H_INCLUDED

#include <math.h>
#include <string.h>

#include <string>
#include <vector>

#include "compiler.h"
#include "compiler_pt3.h"
#include "fswrapper.h"
#include "lex.h"
#include "options.h"
#include "tokenizer.h"

#define ROM_DATA_SIZE COMPILE_CODE_SIZE

using namespace std;

/***
 * @class Rom
 * @brief ROM builder class
 */
class Rom {
 private:
  Tokenizer *tokenizer;
  Compiler *compiler;
  BuildOptions *opts;

  string errorMessage;
  bool errorFound;
  int xbcAddr, pt3Addr;
  int hdrAddr, rtnAddr, mapAddr, txtAddr, filAddr, basAddr, rscAddr;
  int mapInd, txtInd, filInd, basInd;
  int hdrLen, rtnLen, mapLen, txtLen, filLen, pt3Len, basLen;
  int rscSgm, rscLen, codeSgm;

  unsigned char *data;  // [ROM_DATA_SIZE];
  bool writePage[COMPILE_MAX_PAGES];

  vector<unsigned char *> lines;

  // void buildInit(vector<Lexeme*> *resourceList);
  void buildInit();
  void buildHeader();
  void buildRoutines();
  void buildCompilerRoutines();
  void buildCompilerRoutinesPT3();
  void buildMap(vector<Lexeme *> *resourceList, bool font);
  void buildResources(vector<Lexeme *> *resourceList);
  void buildMapAndResources();
  void buildMapAndResourcesText(Lexeme *lexeme);
  void buildMapAndResourcesData(Parser *parser);
  void buildMapAndResourcesFile(Lexeme *lexeme);
  void buildMapAndResourcesFileTXT(string filename);
  void buildMapAndResourcesFileCSV(string filename);
  void buildMapAndResourcesFileSPR(string filename);
  void buildMapAndResourcesFileSCR(string filename);
  void buildMapAndResourcesFileBIN(string filename, string fileext);
  void addResourceToMap(int offset, int length, int filler);
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

  void writeRom(string filename);

 public:
  int code_start, rom_size;
  double stdMemoryPerc, rscMemoryPerc;

  FileNode file;

  Rom();
  ~Rom();

  /***
   * @brief Creates a MSX BASIC ROM based on a pcoded source code
   * @param tokenizer Tokenized source code
   * @return True, if success
   */
  bool build(Tokenizer *tokenizer);

  /***
   * @brief Creates a MSX BASIC ROM based on a compiled source code
   * @param compiler Compiled source code
   * @return True, if success
   */
  bool build(Compiler *compiler);

  /***
   * @brief Creates a MSX BASIC ROM based on a compiled source code usind PT3
   * player
   * @param compiler Compiled PT3 source code
   * @return True, if success
   * @deprecated PT3 is not supported anymore
   */
  bool build(CompilerPT3 *compiler);

  /***
   * @brief Print to the terminal the invalid source code
   */
  void error();
};

#endif  // ASSEMBLY_H_INCLUDED
