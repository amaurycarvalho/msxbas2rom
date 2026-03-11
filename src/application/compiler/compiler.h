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
  unique_ptr<CompilerContext> context;
  unique_ptr<CpuWorkspaceContext> workspace;

 public:
  explicit Compiler(ICpuOpcodeWriter* cpu);
  ~Compiler();

  /***
   * @brief Perform a semanthic analysis on the parsed list
   * @param parser Parser object
   * @return True, if semanthic analysis success
   */
  bool build(Parser* parser);
  int write(unsigned char* dest, int start_address);

  int getCodeSize() const;
  float getRamMemoryPerc() const;
  bool getPt3() const;
  bool getAkm() const;
  bool getFont() const;
  bool getHasTinySprite() const;
  Logger* getLogger();

  SymbolManager* getSymbolManager();
  const SymbolManager* getSymbolManager() const;

  ResourceManager* getResourceManager();
  const ResourceManager* getResourceManager() const;

  int getRamSize() const;

  const string& getErrorMessage() const;

  TagNode* getCurrentTag() const;

  Parser* getParser() const;
  BuildOptions* getOpts() const;

  bool isCompiled() const;
};

#endif  // COMPILER_H
