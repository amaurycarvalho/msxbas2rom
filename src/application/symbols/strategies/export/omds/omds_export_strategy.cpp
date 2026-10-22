/***
 * @file omds_export_strategy.cpp
 * @brief OMDS (*.omds) export strategy implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "omds_export_strategy.h"

#include "build_options.h"
#include "code_node.h"
#include "symbol_export_context.h"
#include "symbol_output_sink.h"

bool OmdsExportStrategy::save(shared_ptr<SymbolExportContext> context,
                              shared_ptr<BuildOptions> opts,
                              shared_ptr<SymbolOutputSink> sink) {
  string output;
  shared_ptr<CodeNode> codeItem;
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
  auto kernelSymbols = context->kernelSymbolAddresses;
  auto& codeList = context->codeList;
  auto& dataList = context->dataList;

  if (!sink) return false;

  context->exportFilename = opts->baseFilename + ".omds";

  {
    output.append(omds_header, sizeof(omds_header) - 1);

    t = kernelSymbols.size();
    for (i = 0; i < t; i++) {
      size = snprintf(s, sizeof(s), omds_format, kernelSymbols[i][2].c_str(),
                      kernelSymbols[i][0].c_str(),
                      stoi(kernelSymbols[i][1].c_str(), nullptr, 16), "");
      if (size <= 0) {
        return false;
      }
      output.append(s, size);
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
          return false;
        }
        output.append(s, size);
      }
    }

    t = dataList.size();

    for (i = 0; i < t; i++) {
      codeItem = dataList[i];
      if (codeItem->debug) {
        size = snprintf(s, sizeof(s), omds_format, "variable",
                        codeItem->name.c_str(), codeItem->addr_within_segm, "");
        if (size <= 0) {
          return false;
        }
        output.append(s, size);
      }
    }

    output.append(omds_footer, sizeof(omds_footer) - 1);

    return sink->writeText(context->exportFilename, output);
  }
  return false;
}
