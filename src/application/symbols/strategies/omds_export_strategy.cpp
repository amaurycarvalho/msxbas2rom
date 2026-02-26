/***
 * @file omds_export_strategy.cpp
 * @brief OMDS (*.omds) export strategy implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "omds_export_strategy.h"

bool OmdsExportStrategy::save(
    const std::vector<std::vector<std::string>>& kernelSymbols,
    const std::vector<CodeNode*>& codeList,
    const std::vector<CodeNode*>& dataList,
                              BuildOptions* opts) {
  FILE* file;
  CodeNode* codeItem;
  int i, t;
  char s[255];
  string segmString;
  const char* omds_header = R"(<!DOCTYPE xomds>
<DebugSession version="0.1">
<Symbols>
)";
  const char* omds_footer = R"(</Symbols>
</DebugSession>
)";
  const char* omds_format =
      "<Symbol><type>%s</type><name>%s</name><value>%i</"
      "value><validSlots>65535</validSlots><validRegisters>3968</"
      "validRegisters><source>0</source><segments>%s</segments></Symbol>\n";

  if ((file = fopen(opts->omdsFilename.c_str(), "w"))) {
    fwrite(omds_header, 1, strlen(omds_header), file);

    t = kernelSymbols.size();
    for (i = 0; i < t; i++) {
      sprintf(s, omds_format, kernelSymbols[i][2].c_str(),
              kernelSymbols[i][0].c_str(),
              stoi(kernelSymbols[i][1].c_str(), nullptr, 16), "");
      fwrite(s, 1, strlen(s), file);
    }

    t = codeList.size();

    for (i = 0; i < t; i++) {
      codeItem = codeList[i];
      if (codeItem->debug) {
        segmString = (opts->megaROM) ? to_string(codeItem->segm) : "";
        sprintf(s, omds_format, "jump", codeItem->name.c_str(),
                codeItem->addr_within_segm, segmString.c_str());
        fwrite(s, 1, strlen(s), file);
      }
    }

    t = dataList.size();

    for (i = 0; i < t; i++) {
      codeItem = dataList[i];
      if (codeItem->debug) {
        sprintf(s, omds_format, "variable", codeItem->name.c_str(),
                codeItem->addr_within_segm, "");
        fwrite(s, 1, strlen(s), file);
      }
    }

    fwrite(omds_footer, 1, strlen(omds_footer), file);

    fclose(file);
    return true;
  }
  return false;
}
