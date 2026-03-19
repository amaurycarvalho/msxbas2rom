/***
 * @file symbol_file_export_strategy.cpp
 * @brief Symbol (*.symbol) export strategy implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "symbol_file_export_strategy.h"

#include "build_options.h"
#include "code_node.h"
#include "symbol_export_context.h"

bool SymbolFileExportStrategy::save(shared_ptr<SymbolExportContext> context,
                                    shared_ptr<BuildOptions> opts) {
  FILE* file;
  shared_ptr<CodeNode> codeItem;
  int i, t, size;
  char s[255];
  const char* symbol_format[] = {"S%i_%s EQU 0%XH\n", "%s EQU 0%XH\n"};
  auto kernelSymbols = context->kernelSymbolAddresses;
  auto& codeList = context->codeList;
  auto& dataList = context->dataList;

  context->exportFilename = opts->baseFilename + ".symbol";

  if ((file = fopen(context->exportFilename.c_str(), "w"))) {
    t = kernelSymbols.size();
    for (i = 0; i < t; i++) {
      size =
          snprintf(s, sizeof(s), symbol_format[1], kernelSymbols[i][0].c_str(),
                   stoi(kernelSymbols[i][1], nullptr, 16));
      if (size <= 0) {
        fclose(file);
        return false;
      }
      fwrite(s, 1, size, file);
    }

    t = codeList.size();

    for (i = 0; i < t; i++) {
      codeItem = codeList[i];
      if (codeItem->debug) {
        if (opts->megaROM) {
          size = snprintf(s, sizeof(s), symbol_format[0], codeItem->segm,
                          codeItem->name.c_str(), codeItem->addr_within_segm);
        } else {
          size = snprintf(s, sizeof(s), symbol_format[1],
                          codeItem->name.c_str(), codeItem->addr_within_segm);
        }
        if (size <= 0) {
          fclose(file);
          return false;
        }
        fwrite(s, 1, size, file);
      }
    }

    t = dataList.size();

    for (i = 0; i < t; i++) {
      codeItem = dataList[i];
      if (codeItem->debug) {
        size = snprintf(s, sizeof(s), symbol_format[1], codeItem->name.c_str(),
                        codeItem->addr_within_segm);
        if (size <= 0) {
          fclose(file);
          return false;
        }
        fwrite(s, 1, size, file);
      }
    }

    fclose(file);
    return true;
  }
  return false;
}
