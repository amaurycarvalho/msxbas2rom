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

#include "kernel_packer.h"

class Compiler;
class BuildOptions;
class KernelModuleLibrary;
class ResourceManager;
class Logger;
class RomArtifact;
class RomWriter;

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
  shared_ptr<RomWriter> writer;
  shared_ptr<KernelModuleLibrary> moduleLibrary;
  shared_ptr<RomArtifact> artifact;
  vector<KernelModulePlacement> overflowPlacements;
  KernelPackingResult packingResult;

  int resourceAddress, resourceSegment;

  /// @brief ROM pages
  vector<vector<unsigned char>> pages;

  /// @brief Error management
  bool errorFound;

  /// @brief Initialize data
  void buildInit();

  /// @brief Add kernel code
  bool addKernel();

  /// @brief Patch start-code calls to relocated kernel routines
  bool patchStartCode();

  //! @brief Fix kernel if Konami format
  bool fixKonamiMapper();

  //! @brief Fix kernel if ASCII16 format
  bool fixAscii16Mapper();

  /// @brief Add compiled code
  bool addCompiledCode();

  /// @brief Add overflow modules to the code page
  bool addOverflowModules();

  /// @brief Add resources
  bool addResources();

  /// @brief Set resource map start address
  void setResourceMapStartAddress();

  /// @brief Assemble the in-memory artifact before persistence
  bool assembleArtifact();

 public:
  int romSize;
  int codeSize, resourcesSize;
  double codeShare, resourcesShare, kernelShare;

  explicit Rom(shared_ptr<RomWriter> writer,
               shared_ptr<KernelModuleLibrary> moduleLibrary = nullptr);
  ~Rom();

  shared_ptr<Logger> getLogger();
  shared_ptr<RomArtifact> getArtifact() const;

  /***
   * @brief Creates a MSX-BASIC ROM based on a compiled source code
   * @param compiler Compiled source code
   * @return True, if success
   */
  bool build(shared_ptr<Compiler> compiler);
};

#endif  // ROM_H_INCLUDED
