/***
 * @file noice_export_strategy.h
 * @brief NoICE (*.noi) export strategy
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef NOICE_EXPORT_STRATEGY_H_INCLUDED
#define NOICE_EXPORT_STRATEGY_H_INCLUDED

#include "symbol_export_strategy.h"

class NoIceExportStrategy : public SymbolExportStrategy {
 public:
  bool save(const std::vector<std::vector<std::string>>& kernelSymbols,
            const std::vector<CodeNode*>& codeList,
            const std::vector<CodeNode*>& dataList,
            BuildOptions* opts);
};

#endif  // NOICE_EXPORT_STRATEGY_H_INCLUDED
