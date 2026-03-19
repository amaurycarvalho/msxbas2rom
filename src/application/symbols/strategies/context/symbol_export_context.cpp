#include "symbol_export_context.h"

#include "code_node.h"

SymbolExportContext::SymbolExportContext() {
  kernelSymbolAddresses = {{"LOADER", "4010", "jump"},
                           {"VAR_CURSEGM", "C023", "variable,byte,1"},
                           {"MR_CALL", "41C8", "jump"},
                           {"MR_CALL_TRAP", "41CB", "jump"},
                           {"MR_CHANGE_SGM", "41CE", "jump"},
                           {"MR_GET_BYTE", "41D1", "jump"},
                           {"MR_GET_DATA", "41D4", "jump"},
                           {"MR_JUMP", "41D7", "jump"}};
}

SymbolExportContext::~SymbolExportContext() = default;

void SymbolExportContext::clear() {
  codeList.clear();
  dataList.clear();
}
