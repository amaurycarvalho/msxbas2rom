/***
 * @file compiler.h
 * @brief Compiler class header for semantic analysis,
 *        specialized as a Z80 code builder for MSX system
 * @author Amaury Carvalho (2019-2025)
 * @note
 *   https://en.wikipedia.org/wiki/Semantic_analysis_(computational)
 *   https://refactoring.guru/design-patterns/bridge
 *   Z80 Opcodes:
 *     http://z80-heaven.wikidot.com/instructions-set
 *     https://clrhome.org/table/
 *   Basic Kun Math Pack:
 *     https://www.msx.org/wiki/Category:X-BASIC#Floating_points
 */

#ifndef COMPILER_H
#define COMPILER_H

#include <string.h>

#include <memory>

#define COMPILE_MAX_PAGES (16 * 4)
#define COMPILE_CODE_SIZE (COMPILE_MAX_PAGES * 0x4000)
#define COMPILE_RAM_SIZE (0xFFFF)

// extern unsigned char bin_header_bin[];
class CompilerContext;
class CpuWorkspaceContext;
class ICpuOpcodeWriter;
class Parser;
class SymbolManager;
class ResourceManager;
class BuildOptions;
class TagNode;
class Logger;

using namespace std;

/***
 * @class Compiler
 * @brief Compiler class for semantic analysis,
 * specialized as a Z80 code builder for MSX system
 */
class Compiler {
 private:
  shared_ptr<CompilerContext> context;
  shared_ptr<CpuWorkspaceContext> workspace;

 public:
  explicit Compiler(shared_ptr<ICpuOpcodeWriter> cpu);
  ~Compiler();

  /***
   * @brief Perform a semanthic analysis on the parsed list
   * @param parser Parser object
   * @return True, if semanthic analysis success
   */
  bool build(shared_ptr<Parser> parser);
  int write(unsigned char* dest, int start_address);

  int getCodeSize() const;
  float getRamMemoryPerc() const;
  bool getPt3() const;
  bool getAkm() const;
  bool getFont() const;
  bool getHasTinySprite() const;
  shared_ptr<Logger> getLogger();

  shared_ptr<SymbolManager> getSymbolManager();
  const shared_ptr<SymbolManager> getSymbolManager() const;

  shared_ptr<ResourceManager> getResourceManager();
  const shared_ptr<ResourceManager> getResourceManager() const;

  int getRamSize() const;

  shared_ptr<TagNode> getCurrentTag() const;

  shared_ptr<Parser> getParser() const;
  shared_ptr<BuildOptions> getOpts() const;

  bool isCompiled() const;
};

#endif  // COMPILER_H
