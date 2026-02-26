/***
 * @file omds_export_strategy.h
 * @brief OMDS (*.omds) export strategy
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef OMDS_EXPORT_STRATEGY_H_INCLUDED
#define OMDS_EXPORT_STRATEGY_H_INCLUDED

#include "symbol_export_strategy.h"

class OmdsExportStrategy : public SymbolExportStrategy {
 public:
  bool save(const std::vector<std::vector<std::string>>& kernelSymbols,
            const std::vector<CodeNode*>& codeList,
            const std::vector<CodeNode*>& dataList,
            BuildOptions* opts);
};

#endif  // OMDS_EXPORT_STRATEGY_H_INCLUDED
