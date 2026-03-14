/***
 * @file elf_export_strategy.h
 * @brief ELF (*.elf) export strategy interface for GDB
 * @author Amaury Carvalho
 * @note
 * Generates a minimal ELF file containing a symbol table usable by GDB.
 * Intended for integration with openMSX GDB server and Visual Studio Code.
 */

#ifndef ELF_EXPORT_STRATEGY_H
#define ELF_EXPORT_STRATEGY_H

#include <string>

#include "symbol_export_strategy.h"

/**
 * @class ElfExportStrategy
 * @brief Strategy responsible for exporting debug symbols in ELF format.
 *
 * This strategy generates a minimal ELF file containing:
 *  - ELF header
 *  - symbol table (.symtab)
 *  - string table (.strtab)
 *
 * The resulting ELF file can be used by the GDB client while debugging
 * a running ROM inside the openMSX GDB server.
 */
class ElfExportStrategy : public SymbolExportStrategy {
 public:
  /**
   * @brief Saves debug symbols to an ELF file.
   *
   * @param symbolManager Pointer to the symbol manager containing
   *                      code and data symbol lists.
   * @param opts Build options containing output filename and build flags.
   *
   * @return true if the file was successfully generated.
   * @return false if an error occurred.
   */
  bool save(SymbolManager* symbolManager, BuildOptions* opts);
};

#endif  // ELF_EXPORT_STRATEGY_H