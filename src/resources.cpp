/***
 * @file resources.cpp
 * @brief Resources class implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resources.h"

void ResourceManager::clear() {
  resourceList.clear();
  codeList.clear();
  fileList.clear();
  dataList.clear();
}

bool ResourceManager::saveSymbolFile(BuildOptions *opts, int code_start,
                                     int ram_page) {
  FILE *file;
  CodeNode *codeItem;
  int i, t, segm, segm2, addr;
  char s[255];

  if ((file = fopen(opts->symbolFilename.c_str(), "w"))) {
    strcpy(s, "LOADER EQU 04010H\t\t; jump\n");
    fwrite(s, 1, strlen(s), file);

    if (opts->megaROM) {
      strcpy(s, "CURSEGM EQU 0C023H\t\t; variable\n");
      fwrite(s, 1, strlen(s), file);
      strcpy(s, "MR_CALL EQU 041C8H\t\t; jump\n");
      fwrite(s, 1, strlen(s), file);
      strcpy(s, "MR_CALL_TRAP EQU 041CBH\t\t; jump\n");
      fwrite(s, 1, strlen(s), file);
      strcpy(s, "MR_CHANGE_SGM EQU 041CEH\t\t; jump\n");
      fwrite(s, 1, strlen(s), file);
      strcpy(s, "MR_GET_BYTE EQU 041D1H\t\t; jump\n");
      fwrite(s, 1, strlen(s), file);
      strcpy(s, "MR_GET_DATA EQU 041D4H\t\t; jump\n");
      fwrite(s, 1, strlen(s), file);
      strcpy(s, "MR_JUMP EQU 041D7H\t\t; jump\n");
      fwrite(s, 1, strlen(s), file);
    }

    /// lines symbols

    t = codeList.size();

    for (i = 0; i < t; i++) {
      codeItem = codeList[i];
      if (codeItem->debug) {
        addr = code_start;
        if (opts->megaROM) {
          segm = codeItem->start / 0x2000 + 2;
          segm2 = codeItem->start / 0x4000;
          addr += (codeItem->start - (segm2 * 0x4000));
          sprintf(s, "%s_S%3i EQU 0%XH\t\t; jump\n", codeItem->name.c_str(),
                  segm, addr);
        } else {
          addr += codeItem->start;
          sprintf(s, "%s EQU 0%XH\t\t; jump\n", codeItem->name.c_str(), addr);
        }
        fwrite(s, 1, strlen(s), file);
      }
    }

    /// variables symbols

    t = dataList.size();

    for (i = 0; i < t; i++) {
      codeItem = dataList[i];
      if (codeItem->debug) {
        sprintf(s, "%s EQU 0%XH\t\t; variable\n", codeItem->name.c_str(),
                codeItem->start + ram_page);
        fwrite(s, 1, strlen(s), file);
      }
    }

    fclose(file);
    return true;
  }
  return false;
}
