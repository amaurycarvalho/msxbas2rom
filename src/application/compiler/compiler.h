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
#include <queue>

#include "compiler_context.h"
#include "compiler_evaluator.h"
#include "cpu_opcode_writer.h"
#include "fswrapper.h"
#include "parser.h"
#include "pletter.h"
#include "resources.h"
#include "symbols.h"

using namespace std;

#define COMPILE_MAX_PAGES (16 * 4)
#define COMPILE_CODE_SIZE (COMPILE_MAX_PAGES * 0x4000)
#define COMPILE_RAM_SIZE (0xFFFF)

extern unsigned char bin_header_bin[];

/***
 * @class Compiler
 * @brief Compiler class for semantic analysis,
 * specialized as a Z80 code builder for MSX system
 */
class Compiler {
 private:
  CompilerContext context;
  unique_ptr<CpuWorkspaceContext> workspace;

 public:
  explicit Compiler(ICpuOpcodeWriter* cpu);

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

  SymbolManager& getSymbolManager();
  const SymbolManager& getSymbolManager() const;

  ResourceManager& getResourceManager();
  const ResourceManager& getResourceManager() const;

  int getRamSize() const;

  const string& getErrorMessage() const;

  TagNode* getCurrentTag() const;

  Parser* getParser() const;
  BuildOptions* getOpts() const;

  bool isCompiled() const;
};

#endif  // COMPILER_H
