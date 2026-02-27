/***
 * @file symbol_file_export_strategy.cpp
 * @brief Symbol (*.symbol) export strategy implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "symbol_file_export_strategy.h"

bool SymbolFileExportStrategy::save(SymbolManager* symbolManager,
                                    BuildOptions* opts) {
  FILE* file;
  CodeNode* codeItem;
  int i, t;
  char s[255];
  const char* symbol_format[] = {"S%i_%s EQU 0%XH\n", "%s EQU 0%XH\n"};
  vector<vector<string>> kernelSymbols =
      symbolManager->getKernelSymbolAddresses();
  vector<CodeNode*>& codeList = symbolManager->codeList;
  vector<CodeNode*>& dataList = symbolManager->dataList;

  if ((file = fopen(opts->symbolFilename.c_str(), "w"))) {
    t = kernelSymbols.size();
    for (i = 0; i < t; i++) {
      snprintf(s, sizeof(s), symbol_format[1], kernelSymbols[i][0].c_str(),
               stoi(kernelSymbols[i][1], nullptr, 16));
      fwrite(s, 1, strlen(s), file);
    }

    t = codeList.size();

    for (i = 0; i < t; i++) {
      codeItem = codeList[i];
      if (codeItem->debug) {
        if (opts->megaROM) {
          snprintf(s, sizeof(s), symbol_format[0], codeItem->segm,
                   codeItem->name.c_str(), codeItem->addr_within_segm);
        } else {
          snprintf(s, sizeof(s), symbol_format[1], codeItem->name.c_str(),
                   codeItem->addr_within_segm);
        }
        fwrite(s, 1, strlen(s), file);
      }
    }

    t = dataList.size();

    for (i = 0; i < t; i++) {
      codeItem = dataList[i];
      if (codeItem->debug) {
        snprintf(s, sizeof(s), symbol_format[1], codeItem->name.c_str(),
                 codeItem->addr_within_segm);
        fwrite(s, 1, strlen(s), file);
      }
    }

    fclose(file);
    return true;
  }
  return false;
}
