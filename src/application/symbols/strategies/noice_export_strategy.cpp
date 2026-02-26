/***
 * @file noice_export_strategy.cpp
 * @brief NoICE (*.noi) export strategy implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "noice_export_strategy.h"

#include "symbol_manager.h"

bool NoIceExportStrategy::save(SymbolManager* symbolManager,
                               BuildOptions* opts) {
  FILE* file;
  CodeNode* codeItem;
  int i, t;
  char s[255];
  const char* noice_format = "def %s %XH  ; %s\n";
  std::vector<std::vector<std::string>> kernelSymbols =
      symbolManager->getKernelSymbolAddresses();
  std::vector<CodeNode*>& codeList = symbolManager->codeList;
  std::vector<CodeNode*>& dataList = symbolManager->dataList;

  if ((file = fopen(opts->noiceFilename.c_str(), "w"))) {
    t = kernelSymbols.size();
    for (i = 0; i < t; i++) {
      sprintf(s, noice_format, kernelSymbols[i][0].c_str(),
              stoi(kernelSymbols[i][1], nullptr, 16),
              kernelSymbols[i][2].c_str());
      fwrite(s, 1, strlen(s), file);
    }

    t = codeList.size();

    for (i = 0; i < t; i++) {
      codeItem = codeList[i];
      if (codeItem->debug) {
        sprintf(s, noice_format, codeItem->name.c_str(),
                codeItem->segm << 16 | codeItem->addr_within_segm, "jump");
        fwrite(s, 1, strlen(s), file);
      }
    }

    t = dataList.size();

    for (i = 0; i < t; i++) {
      codeItem = dataList[i];
      if (codeItem->debug) {
        sprintf(s, noice_format, codeItem->name.c_str(),
                codeItem->segm << 16 | codeItem->addr_within_segm, "variable");
        fwrite(s, 1, strlen(s), file);
      }
    }

    fclose(file);
    return true;
  }
  return false;
}
