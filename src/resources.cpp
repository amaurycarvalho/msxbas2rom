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

void ResourceManager::addDataResource(Parser *parser) {
  Lexeme *lexeme = new Lexeme();
  lexeme->name = "_DATA_";
  lexeme->value = lexeme->name;
  resourceList.push_back(lexeme);

  ResourceDataReader *resourceReader = new ResourceDataReader(parser);
  if (resourceReader) {
    resourceReaderList.push_back(resourceReader);
    resourceReader->load();
  }
}

bool ResourceManager::add(string filename) {
  ResourceReader *resourceReader = ResourceReader::create(filename);
  if (resourceReader) {
    resourceReaderList.push_back(resourceReader);
    return resourceReader->load();
  }
  return false;
}

bool ResourceManager::saveSymbolFile(BuildOptions *opts) {
  FILE *file;
  CodeNode *codeItem;
  int i, t;
  char s[255];
  const char *symbol_format[] = {"S%i_%s EQU 0%XH\n", "%s EQU 0%XH\n"};
  vector<vector<string>> symbol_values = {
      {"LOADER", "4010"},        {"CURSEGM", "C023"},
      {"MR_CALL", "41C8"},       {"MR_CALL_TRAP", "41CB"},
      {"MR_CHANGE_SGM", "41CE"}, {"MR_GET_BYTE", "41D1"},
      {"MR_GET_DATA", "41D4"},   {"MR_JUMP", "41D7"}};

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

bool ResourceManager::saveOmdsFile(BuildOptions *opts) {
  FILE *file;
  CodeNode *codeItem;
  int i, t;
  char s[255];
  string segmString;
  const char *omds_header = R"(<!DOCTYPE xomds>
<DebugSession version="0.1">
<Symbols>
)";
  const char *omds_footer = R"(</Symbols>
</DebugSession>
)";
  const char *omds_format =
      "<Symbol><type>%s</type><name>%s</name><value>%i</"
      "value><validSlots>65535</validSlots><validRegisters>3968</"
      "validRegisters><source>0</source><segments>%s</segments></Symbol>\n";
  vector<vector<string>> omds_values = {
      {"jump", "LOADER", "4010"},        {"variable", "CURSEGM", "C023"},
      {"jump", "MR_CALL", "41C8"},       {"jump", "MR_CALL_TRAP", "41CB"},
      {"jump", "MR_CHANGE_SGM", "41CE"}, {"jump", "MR_GET_BYTE", "41D1"},
      {"jump", "MR_GET_DATA", "41D4"},   {"jump", "MR_JUMP", "41D7"}};

  if ((file = fopen(opts->omdsFilename.c_str(), "w"))) {
    fwrite(omds_header, 1, strlen(omds_header), file);

    t = omds_values.size();
    for (i = 0; i < t; i++) {
      sprintf(s, omds_format, omds_values[i][0].c_str(),
              omds_values[i][1].c_str(),
              stoi(omds_values[i][2].c_str(), nullptr, 16), "");
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

///-------------------------------------------------------------------------------

ResourceReader::ResourceReader(string filename) {
  this->filename = filename;
};

bool ResourceReader::isValid(string fileext) {
  return false;
}

bool ResourceReader::load() {
  return false;
}

ResourceReader *ResourceReader::create(string filename) {
  if (fileExists(filename)) {
    string fileext = getFileExtension(filename);
    if (ResourceTxtReader::isValid(fileext)) {
      return new ResourceTxtReader(filename);
    }
    if (ResourceCsvReader::isValid(fileext)) {
      return new ResourceCsvReader(filename);
    }
    if (ResourceScrReader::isValid(fileext)) {
      return new ResourceScrReader(filename);
    }
    if (ResourceSprReader::isValid(fileext)) {
      return new ResourceSprReader(filename);
    }
    if (ResourceAkmReader::isValid(fileext)) {
      return new ResourceAkmReader(filename);
    }
    if (ResourceAkxReader::isValid(fileext)) {
      return new ResourceAkxReader(filename);
    }
    if (ResourceMtfReader::isValid(fileext)) {
      return new ResourceMtfReader(filename);
    }
    if (ResourceBlobReader::isValid(fileext)) {
      return new ResourceBlobReader(filename);
    }
  }
  return nullptr;
}

ResourceBlobReader::ResourceBlobReader(string filename)
    : ResourceReader(filename) {};

bool ResourceBlobReader::isValid(string fileext) {
  return true;
}

bool ResourceBlobReader::load() {
  return false;
}

ResourceTxtReader::ResourceTxtReader(string filename)
    : ResourceReader(filename) {};

bool ResourceTxtReader::isValid(string fileext) {
  return (strcasecmp(fileext.c_str(), ".TXT") == 0);
}

bool ResourceTxtReader::load() {
  return false;
}

ResourceCsvReader::ResourceCsvReader(string filename)
    : ResourceReader(filename) {};

bool ResourceCsvReader::isValid(string fileext) {
  return (strcasecmp(fileext.c_str(), ".CSV") == 0);
}

bool ResourceCsvReader::load() {
  return false;
}

ResourceScrReader::ResourceScrReader(string filename)
    : ResourceReader(filename) {};

bool ResourceScrReader::isValid(string fileext) {
  return (strcasecmp(fileext.c_str(), ".SC0") == 0 ||
          strcasecmp(fileext.c_str(), ".SC1") == 0 ||
          strcasecmp(fileext.c_str(), ".SC2") == 0 ||
          strcasecmp(fileext.c_str(), ".SC3") == 0 ||
          strcasecmp(fileext.c_str(), ".SC4") == 0 ||
          strcasecmp(fileext.c_str(), ".SC5") == 0 ||
          strcasecmp(fileext.c_str(), ".SC6") == 0 ||
          strcasecmp(fileext.c_str(), ".SC7") == 0 ||
          strcasecmp(fileext.c_str(), ".SC8") == 0 ||
          strcasecmp(fileext.c_str(), ".SC9") == 0 ||
          strcasecmp(fileext.c_str(), ".S10") == 0 ||
          strcasecmp(fileext.c_str(), ".S11") == 0 ||
          strcasecmp(fileext.c_str(), ".S12") == 0);
}

bool ResourceScrReader::load() {
  return false;
}

ResourceSprReader::ResourceSprReader(string filename)
    : ResourceReader(filename) {};

bool ResourceSprReader::isValid(string fileext) {
  return (strcasecmp(fileext.c_str(), ".SPR") == 0);
}

bool ResourceSprReader::load() {
  return false;
}

ResourceAkmReader::ResourceAkmReader(string filename)
    : ResourceBlobReader(filename) {};

bool ResourceAkmReader::isValid(string fileext) {
  return (strcasecmp(fileext.c_str(), ".AKM") == 0);
}

bool ResourceAkmReader::load() {
  if (ResourceBlobReader::load()) {
    fix();
    return true;
  }
  return false;
}

void ResourceAkmReader::fix() {
  return;
}

ResourceAkxReader::ResourceAkxReader(string filename)
    : ResourceBlobReader(filename) {};

bool ResourceAkxReader::isValid(string fileext) {
  return (strcasecmp(fileext.c_str(), ".AKX") == 0);
}

bool ResourceAkxReader::load() {
  if (ResourceBlobReader::load()) {
    fix();
    return true;
  }
  return false;
}

void ResourceAkxReader::fix() {
  return;
}

ResourceMtfReader::ResourceMtfReader(string filename)
    : ResourceBlobReader(filename) {};

bool ResourceMtfReader::isValid(string fileext) {
  return (strcasecmp(fileext.c_str(), ".MTF.JSON") == 0);
}

bool ResourceMtfReader::load() {
  return false;
}

ResourceDataReader::ResourceDataReader(Parser *parser)
    : ResourceReader(string("_DATA_")) {
  this->parser = parser;
};

bool ResourceDataReader::load() {
  return false;
}
