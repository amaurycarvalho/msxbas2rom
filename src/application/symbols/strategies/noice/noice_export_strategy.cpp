/***
 * @file noice_export_strategy.cpp
 * @brief NoICE (*.noi) export strategy implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "noice_export_strategy.h"

bool NoIceExportStrategy::save(SymbolManager* symbolManager,
                               BuildOptions* opts) {
  FILE* file;
  CodeNode* codeItem;
  int i, t;
  char s[255];
  const char* noice_format = "def %s %XH  ; %s\n";
  string comment;
  vector<vector<string>> kernelSymbols =
      symbolManager->getKernelSymbolAddresses();
  vector<CodeNode*>& codeList = symbolManager->codeList;
  vector<CodeNode*>& dataList = symbolManager->dataList;

  if ((file = fopen(opts->noiceFilename.c_str(), "w"))) {
    t = kernelSymbols.size();
    for (i = 0; i < t; i++) {
      snprintf(s, sizeof(s), noice_format, kernelSymbols[i][0].c_str(),
               stoi(kernelSymbols[i][1], nullptr, 16),
               kernelSymbols[i][2].c_str());
      fwrite(s, 1, strlen(s), file);
    }

    t = codeList.size();
    comment = "jump";

    for (i = 0; i < t; i++) {
      codeItem = codeList[i];
      if (codeItem->debug) {
        snprintf(s, sizeof(s), noice_format, codeItem->name.c_str(),
                 codeItem->segm << 16 | codeItem->addr_within_segm,
                 comment.c_str());
        fwrite(s, 1, strlen(s), file);
      }
    }

    t = dataList.size();

    for (i = 0; i < t; i++) {
      codeItem = dataList[i];
      if (codeItem->debug) {
        comment = "variable";
        if (codeItem->lexeme) {
          switch (codeItem->lexeme->subtype) {
            case Lexeme::subtype_numeric:
              comment += ",integer,";
              break;
            case Lexeme::subtype_single_decimal:
              comment += ",single(float),";
              break;
            case Lexeme::subtype_double_decimal:
              comment += ",double(float),";
              break;
            case Lexeme::subtype_string:
            case Lexeme::subtype_basic_string:
              comment += ",string(pascal),";
              break;
            default:
              comment += ",unidentified,";
              break;
          }
          comment += to_string(codeItem->length);
          if (codeItem->lexeme->isArray) {
            comment += ",array";
            if (codeItem->lexeme->x_size)
              comment += "," + to_string(codeItem->lexeme->x_size);
            if (codeItem->lexeme->y_size)
              comment += "," + to_string(codeItem->lexeme->y_size);
          }
        }
        snprintf(s, sizeof(s), noice_format, codeItem->name.c_str(),
                 codeItem->segm << 16 | codeItem->addr_within_segm,
                 comment.c_str());
        fwrite(s, 1, strlen(s), file);
      }
    }

    fclose(file);
    return true;
  }
  return false;
}
