/***
 * @file symbol_file_export_strategy.h
 * @brief Symbol (*.symbol) export strategy
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef SYMBOL_FILE_EXPORT_STRATEGY_H_INCLUDED
#define SYMBOL_FILE_EXPORT_STRATEGY_H_INCLUDED

#include "symbol_export_strategy.h"

class SymbolFileExportStrategy : public SymbolExportStrategy {
 public:
  bool save(const std::vector<std::vector<std::string>>& kernelSymbols,
            const std::vector<CodeNode*>& codeList,
            const std::vector<CodeNode*>& dataList,
            BuildOptions* opts);
};

#endif  // SYMBOL_FILE_EXPORT_STRATEGY_H_INCLUDED
