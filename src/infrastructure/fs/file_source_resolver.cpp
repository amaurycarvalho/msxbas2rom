/***
 * @file file_source_resolver.cpp
 * @brief File-backed source resolver adapter implementation
 * @author Amaury Carvalho (2026)
 */

#include "file_source_resolver.h"

#include <cstdio>
#include <fstream>

bool FileSourceResolver::validateHeader(const std::string& name,
                                        const std::string& content) {
  unsigned char header[3] = {0, 0, 0};
  char message[512];

  if (content.empty()) {
    error = "Empty file";
    return false;
  }

  std::size_t headerSize = content.size() < 3 ? content.size() : 3;
  for (std::size_t i = 0; i < headerSize; i++) {
    header[i] = static_cast<unsigned char>(content[i]);
  }

  if (header[0] < 0x20 || header[0] > 126) {
    if (header[0] == 0xFF && header[2] == 0x80) {
      snprintf(message, sizeof(message),
               "Tokenized MSX-BASIC source code file detected\nSave it as a "
               "plain text to use it "
               "with MSXBAS2ROM:\nSAVE \"%s\",A",
               name.c_str());
      error = message;
      return false;
    }
    if (header[0] != 0x0D && header[0] != 0x0A && header[0] != 0x0C) {
      error = "This is not a MSX-BASIC source code file.";
      return false;
    }
  }

  return true;
}

bool FileSourceResolver::resolve(const std::string& name,
                                 SourceDocument& document) {
  std::ifstream file(name.c_str(), std::ios::binary);
  if (!file) {
    error = "File doesn't exist";
    return false;
  }

  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  if (!validateHeader(name, content)) return false;

  document.logicalName = name;
  document.content = content;
  error.clear();
  return true;
}

std::string FileSourceResolver::lastError() const {
  return error;
}
