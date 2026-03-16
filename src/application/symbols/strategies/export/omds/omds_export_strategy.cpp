/***
 * @file omds_export_strategy.cpp
 * @brief OMDS (*.omds) export strategy implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "omds_export_strategy.h"

#include "build_options.h"
#include "code_node.h"
#include "symbol_manager.h"

bool OmdsExportStrategy::save(SymbolManager* symbolManager,
                              BuildOptions* opts) {
  FILE* file;
  CodeNode* codeItem;
  int i, t, size;
  char s[255];
  string segmString;
  constexpr const char omds_header[] = R"(<!DOCTYPE xomds>
<DebugSession version="0.1">
<Symbols>
)";
  constexpr const char omds_footer[] = R"(</Symbols>
</DebugSession>
)";
  constexpr const char omds_format[] =
      "<Symbol><type>%s</type><name>%s</name><value>%i</"
      "value><validSlots>65535</validSlots><validRegisters>3968</"
      "validRegisters><source>0</source><segments>%s</segments></Symbol>\n";
  auto kernelSymbols = symbolManager->getKernelSymbolAddresses();
  auto& codeList = symbolManager->codeList;
  auto& dataList = symbolManager->dataList;

  symbolManager->exportFilename = opts->baseFilename + ".omds";

  if ((file = fopen(symbolManager->exportFilename.c_str(), "w"))) {
    fwrite(omds_header, 1, sizeof(omds_header) - 1, file);

    t = kernelSymbols.size();
    for (i = 0; i < t; i++) {
      size = snprintf(s, sizeof(s), omds_format, kernelSymbols[i][2].c_str(),
                      kernelSymbols[i][0].c_str(),
                      stoi(kernelSymbols[i][1].c_str(), nullptr, 16), "");
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
        segmString = (opts->megaROM) ? to_string(codeItem->segm) : "";
        size =
            snprintf(s, sizeof(s), omds_format, "jump", codeItem->name.c_str(),
                     codeItem->addr_within_segm, segmString.c_str());
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
        size = snprintf(s, sizeof(s), omds_format, "variable",
                        codeItem->name.c_str(), codeItem->addr_within_segm, "");
        if (size <= 0) {
          fclose(file);
          return false;
        }
        fwrite(s, 1, size, file);
      }
    }

    fwrite(omds_footer, 1, sizeof(omds_footer) - 1, file);

    fclose(file);
    return true;
  }
  return false;
}
