/***
 * @file source_document.cpp
 * @brief Application source document helpers
 * @author Amaury Carvalho (2026)
 */

#include "source_document.h"

#include <algorithm>

std::vector<std::string> splitSourceLines(const std::string& content) {
  std::vector<std::string> lines;
  std::size_t offset = 0;

  while (offset < content.size()) {
    std::size_t limit = std::min(content.size(), offset + 254);
    std::size_t end = limit;

    for (std::size_t i = offset; i < limit; i++) {
      if (content[i] == '\n') {
        end = i + 1;
        break;
      }
    }

    lines.push_back(content.substr(offset, end - offset));
    offset = end;
  }

  return lines;
}
