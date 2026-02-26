/***
 * @file symbols.cpp
 * @brief Symbols class implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "symbols.h"

void SymbolManager::clear() {
  codeList.clear();
  dataList.clear();
}

vector<vector<string>> SymbolManager::getKernelSymbolAddresses() {
  return {
      {"LOADER", "4010", "jump"},        {"VAR_CURSEGM", "C023", "variable"},
      {"MR_CALL", "41C8", "jump"},       {"MR_CALL_TRAP", "41CB", "jump"},
      {"MR_CHANGE_SGM", "41CE", "jump"}, {"MR_GET_BYTE", "41D1", "jump"},
      {"MR_GET_DATA", "41D4", "jump"},   {"MR_JUMP", "41D7", "jump"}};
}

bool SymbolManager::saveSymbolFile(BuildOptions* opts) {
  FILE* file;
  CodeNode* codeItem;
  int i, t;
  char s[255];
  const char* symbol_format[] = {"S%i_%s EQU 0%XH\n", "%s EQU 0%XH\n"};
  vector<vector<string>> symbol_values = getKernelSymbolAddresses();

  if ((file = fopen(opts->symbolFilename.c_str(), "w"))) {
    t = symbol_values.size();
    for (i = 0; i < t; i++) {
      sprintf(s, symbol_format[1], symbol_values[i][0].c_str(),
              stoi(symbol_values[i][1], nullptr, 16));
      fwrite(s, 1, strlen(s), file);
    }

    /// lines symbols

    t = codeList.size();

    for (i = 0; i < t; i++) {
      codeItem = codeList[i];
      if (codeItem->debug) {
        if (opts->megaROM) {
          sprintf(s, symbol_format[0], codeItem->segm, codeItem->name.c_str(),
                  codeItem->addr_within_segm);
        } else {
          sprintf(s, symbol_format[1], codeItem->name.c_str(),
                  codeItem->addr_within_segm);
        }
        fwrite(s, 1, strlen(s), file);
      }
    }

    /// variables symbols

    t = dataList.size();

    for (i = 0; i < t; i++) {
      codeItem = dataList[i];
      if (codeItem->debug) {
        sprintf(s, symbol_format[1], codeItem->name.c_str(),
                codeItem->addr_within_segm);
        fwrite(s, 1, strlen(s), file);
      }
    }

    fclose(file);
    return true;
  }
  return false;
}

bool SymbolManager::saveNoIceFile(BuildOptions* opts) {
  FILE* file;
  CodeNode* codeItem;
  int i, t;
  char s[255];
  const char* noice_format = "def %s %XH  ; %s\n";
  vector<vector<string>> noice_values = getKernelSymbolAddresses();

  if ((file = fopen(opts->noiceFilename.c_str(), "w"))) {
    t = noice_values.size();
    for (i = 0; i < t; i++) {
      sprintf(s, noice_format, noice_values[i][0].c_str(),
              stoi(noice_values[i][1], nullptr, 16),
              noice_values[i][2].c_str());
      fwrite(s, 1, strlen(s), file);
    }

    /// lines symbols

    t = codeList.size();

    for (i = 0; i < t; i++) {
      codeItem = codeList[i];
      if (codeItem->debug) {
        sprintf(s, noice_format, codeItem->name.c_str(),
                codeItem->segm << 16 | codeItem->addr_within_segm, "jump");
        fwrite(s, 1, strlen(s), file);
      }
    }

    /// variables symbols

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

bool SymbolManager::saveOmdsFile(BuildOptions* opts) {
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
  vector<vector<string>> omds_values = getKernelSymbolAddresses();

  if ((file = fopen(opts->omdsFilename.c_str(), "w"))) {
    fwrite(omds_header, 1, strlen(omds_header), file);

    t = omds_values.size();
    for (i = 0; i < t; i++) {
      sprintf(s, omds_format, omds_values[i][2].c_str(),
              omds_values[i][0].c_str(),
              stoi(omds_values[i][1].c_str(), nullptr, 16), "");
      fwrite(s, 1, strlen(s), file);
    }

    /// lines symbols

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

    /// variables symbols

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
