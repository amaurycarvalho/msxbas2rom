/***
 * @file symbol_export_strategy.h
 * @brief Symbol export strategy interface
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef SYMBOL_EXPORT_STRATEGY_H_INCLUDED
#define SYMBOL_EXPORT_STRATEGY_H_INCLUDED

#include <string>
#include <vector>

#include "code_node.h"
#include "options.h"

class SymbolExportStrategy {
 public:
  virtual ~SymbolExportStrategy() {}

  virtual bool save(const std::vector<std::vector<std::string>>& kernelSymbols,
                    const std::vector<CodeNode*>& codeList,
                    const std::vector<CodeNode*>& dataList,
                    BuildOptions* opts) = 0;
};

#endif  // SYMBOL_EXPORT_STRATEGY_H_INCLUDED
