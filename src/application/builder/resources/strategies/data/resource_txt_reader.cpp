/***
 * @file resource_txt_reader.cpp
 * @brief Resource text reader implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_txt_reader.h"

#include <algorithm>
#include <fstream>
#include <string.h>
#include <strings.h>

ResourceTxtReader::ResourceTxtReader(string filename)
    : ResourceReader(filename) {
  /// @todo Refactor it implementing resource block linked list
  has1stBlockAnd2ndBlockSegmentDisalignmentBug = true;
};

bool ResourceTxtReader::populateLines() {
  ifstream file(filename);
  if (!file) {
    errorMessage = "Resource file not found: " + filename;
    return false;
  }

  lines.clear();
  string line;

  while (getline(file, line)) {
    // remove any CR/LF from the end of string
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
      line.pop_back();
    }
    // replace non-printable / non-ASCII characters with spaces
    for (char &ch : line) {
      if (ch < 32) {
        ch = ' ';
      }
    }
    lines.push_back(line);
  }

  return true;
}

bool ResourceTxtReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".TXT") == 0);
}

/// @todo fix 1st block segment disalignment bug
/// implementing resource block linked list
bool ResourceTxtReader::load() {
  int i, lineCount, lineSize;
  data.clear();
  if (populateLines()) {
    unpackedSize = 1;
    packedSize = 1;
    lineCount = lines.size();
    data.emplace_back(1);
    data.back()[0] = 2;  //! TXT data resource
    for (i = 0; i < lineCount; i++) {
      /// string maximum size = 255 chars
      lineSize = min((int)lines[i].size(), 255);
      /// add new string element
      data.emplace_back(lineSize + 1);
      data.back()[0] = lineSize;
      /// copy string text (max = 255 chars)
      memcpy(data.back().data() + 1, lines[i].c_str(), lineSize);
      packedSize += lineSize + 1;
      unpackedSize += lineSize + 1;
    }
    return true;
  }
  return false;
}
