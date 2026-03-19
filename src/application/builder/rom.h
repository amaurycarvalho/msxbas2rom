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

#include <memory>
#include <string>
#include <vector>

class Compiler;
class BuildOptions;
class ResourceManager;
class Logger;

using namespace std;

/***
 * @class Rom
 * @brief ROM builder class
 */
class Rom {
 private:
  shared_ptr<Compiler> compiler;
  shared_ptr<BuildOptions> opts;
  shared_ptr<ResourceManager> resourceManager;
  shared_ptr<Logger> logger;

  int resourceAddress, resourceSegment;

  /// @brief ROM pages
  vector<vector<unsigned char>> pages;

  /// @brief Error management
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

  shared_ptr<Logger> getLogger();

  /***
   * @brief Creates a MSX BASIC ROM based on a compiled source code
   * @param compiler Compiled source code
   * @return True, if success
   */
  bool build(shared_ptr<Compiler> compiler);
};

#endif  // ROM_H_INCLUDED
