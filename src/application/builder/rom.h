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
#include "fswrapper.h"
#include "lexer.h"
#include "options.h"
#include "resources.h"

using namespace std;

/***
 * @class Rom
 * @brief ROM builder class
 */
class Rom {
 private:
  Compiler *compiler;
  BuildOptions *opts;
  ResourceManager *resourceManager;

  int resourceAddress, resourceSegment;

  /// @brief ROM pages
  vector<vector<unsigned char>> pages;

  /// @brief Error management
  string errorMessage;
  bool errorFound;

  /// @brief Initialize data
  void buildInit();

  /// @brief Add kernel code
  bool addKernel();

  /// @brief Fix kernel if Konami SCC format
  bool fixIfKonamiSCC();

  /// @brief Add compiled code
  bool addCompiledCode();

  /// @brief Add resources
  bool addResources();

  /// @brief Set resource map start address
  void setResourceMapStartAddress();

  /// @brief Write pages to ROM file
  bool writeRom(string filename);

 public:
  int romSize;
  int codeSize, resourcesSize;
  double codeShare, resourcesShare, kernelShare;

  Rom();
  ~Rom();

  /***
   * @brief Creates a MSX BASIC ROM based on a compiled source code
   * @param compiler Compiled source code
   * @return True, if success
   */
  bool build(Compiler *compiler);

  /***
   * @brief Print to the terminal the invalid source code
   */
  void error();
};

#endif  // ROM_H_INCLUDED
