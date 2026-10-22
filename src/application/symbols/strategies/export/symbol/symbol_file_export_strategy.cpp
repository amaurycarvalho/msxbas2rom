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
#include "symbol_output_sink.h"

bool SymbolFileExportStrategy::save(shared_ptr<SymbolExportContext> context,
                                    shared_ptr<BuildOptions> opts,
                                    shared_ptr<SymbolOutputSink> sink) {
  string output;
  shared_ptr<CodeNode> codeItem;
  int i, t, size;
  char s[255];
  const char* symbol_format[] = {"S%i_%s EQU 0%XH\n", "%s EQU 0%XH\n"};
  auto kernelSymbols = context->kernelSymbolAddresses;
  auto& codeList = context->codeList;
  auto& dataList = context->dataList;

  if (!sink) return false;

  context->exportFilename = opts->baseFilename + ".symbol";

  {
    t = kernelSymbols.size();
    for (i = 0; i < t; i++) {
      size =
          snprintf(s, sizeof(s), symbol_format[1], kernelSymbols[i][0].c_str(),
                   stoi(kernelSymbols[i][1], nullptr, 16));
      if (size <= 0) {
        return false;
      }
      output.append(s, size);
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
          return false;
        }
        output.append(s, size);
      }
    }

    t = dataList.size();

    for (i = 0; i < t; i++) {
      codeItem = dataList[i];
      if (codeItem->debug) {
        size = snprintf(s, sizeof(s), symbol_format[1], codeItem->name.c_str(),
                        codeItem->addr_within_segm);
        if (size <= 0) {
          return false;
        }
        output.append(s, size);
      }
    }

    return sink->writeText(context->exportFilename, output);
  }
  return false;
}
