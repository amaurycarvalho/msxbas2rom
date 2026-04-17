#include "symbol_export_context.h"

#include "code_node.h"

SymbolExportContext::SymbolExportContext() {
  kernelSymbolAddresses = {{"LOADER", "4010", "jump"},
                           {"VAR_CURSEGM", "C023", "variable,byte,1"}};
}

SymbolExportContext::~SymbolExportContext() = default;

void SymbolExportContext::clear() {
  codeList.clear();
  dataList.clear();
}
