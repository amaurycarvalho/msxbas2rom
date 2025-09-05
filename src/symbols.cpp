/***
 * @file symbols.cpp
 * @brief Symbols class implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "symbols.h"

/***
 * @name FileNode class code
 */

FileNode::~FileNode() {
  close();
}

bool FileNode::open() {
  this->clear();
  handle = fopen(name.c_str(), "rb");
  return handle;
}

bool FileNode::eof() {
  if (handle) {
    return feof(handle);
  } else
    return true;
}

int FileNode::read(unsigned char* data, int max_length) {
  bytes = 0;
  memset(data, 0, max_length);
  if (handle) {
    bytes = fread(data, 1, max_length, handle);
    length += bytes;
  }
  return bytes;
}

int FileNode::read() {
  buffer = buf_plain;
  if (read(buffer, 200)) {
    if (packed) {
      memset(buf_packed, 0, 1024);
      bytes = pletter.pack(buffer, 200, &buf_packed[0]);
      buffer = buf_packed;

      packed_length += bytes;
    }
    blocks++;
  }
  return bytes;
}

int FileNode::readAsLexeme() {
  if (read(buf_plain, 200)) {
    readAsLexeme(&buf_plain[0], bytes);
  }

  return bytes;
}

int FileNode::readAsLexeme(unsigned char* data, int data_length) {
  if (packed) {
    memset(buf_packed, 0, 1024);
    bytes = pletter.pack(data, data_length, &buf_packed[0]);

    packed_length += bytes;

    if (bytes == 0 || bytes > 255) return bytes;
  }

  current_lexeme = new Lexeme(Lexeme::type_literal, Lexeme::subtype_binary_data,
                              "FB_" + name + "_" + to_string(blocks), "");
  current_lexeme->value.resize(bytes);

  s = (unsigned char*)current_lexeme->value.data();

  if (packed) {
    memcpy(s, &buf_packed[0], bytes);
  } else {
    memcpy(s, data, data_length);
  }

  if (first) {
    first_lexeme = current_lexeme;
    memcpy(&file_header[0], data, 255);
    first = false;
  }

  blocks++;

  return bytes;
}

void FileNode::close() {
  if (handle) {
    fclose(handle);
    handle = 0;
  }
}

void FileNode::clear() {
  handle = 0;
  length = 0;
  first = true;
  packed_length = 0;
  blocks = 0;
  first_lexeme = 0;
  current_lexeme = 0;
}

bool FileNode::create() {
  clear();
  handle = fopen(name.c_str(), "wb");
  return handle;
}

void FileNode::write(unsigned char* data, int data_length) {
  fwrite(data, 1, data_length, handle);
  length += data_length;
}

//----------------------------------------------------------------------------------------------

string FileNode::stripQuotes(const string& text) {
  string s = text;

  // Remove leading quote
  if (!s.empty() && s.front() == '"') {
    s.erase(s.begin());
  }

  // Remove trailing quote
  if (!s.empty() && s.back() == '"') {
    s.pop_back();
  }

  return s;
}

/*
void FileNode::stripQuotes(string text, char* buf, int buflen) {
  int tt;
  char* s;

  /// strips quotes from text
  tt = text.size();
  s = (char*)text.c_str();
  if (s[0] == '"') {
    s++;
    tt--;
  }
  strncpy(buf, s, buflen);
  if (buf[tt - 1] == '"') {
    buf[tt - 1] = 0;
    tt--;
  }
}
  */

string FileNode::toUpper(const string& input) {
  string result = input;
  transform(result.begin(), result.end(), result.begin(),
            [](unsigned char c) { return toupper(c); });
  return result;
}

string FileNode::getFileExt(string filename) {
  return toUpper(getFileExtension(filename));
}

string FileNode::getFileExt() {
  return getFileExt(name);
}

//----------------------------------------------------------------------------------------------

bool FileNode::writeToFile(string filename, unsigned char* data,
                           int data_length) {
  this->name = filename;

  if (!create()) {
    return false;
  }

  write(data, data_length);

  close();

  return true;
}

int FileNode::readFromFile(string filename, unsigned char* data, int maxlen) {
  FILE* file;
  int total_bytes = 0;

  if ((file = fopen(filename.c_str(), "rb"))) {
    total_bytes = fread(data, 1, maxlen, file);
    fclose(file);
  }

  length = total_bytes;

  return total_bytes;
}

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
