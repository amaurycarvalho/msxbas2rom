/***
 * @file in_memory_resource_content_source.cpp
 * @brief In-memory resource-content source implementation
 * @author Amaury Carvalho (2026)
 */

#include "in_memory_resource_content_source.h"

void InMemoryResourceContentSource::addBinary(
    const std::string& filename, const std::vector<unsigned char>& content) {
  binaryContents[filename] = content;
}

void InMemoryResourceContentSource::addText(const std::string& filename,
                                            const std::string& content) {
  textContents[filename] = content;
}

void InMemoryResourceContentSource::clear() {
  binaryContents.clear();
  textContents.clear();
}

bool InMemoryResourceContentSource::readBinary(
    const std::string& filename, std::vector<unsigned char>& content,
    std::string& error) {
  auto it = binaryContents.find(filename);
  if (it == binaryContents.end()) {
    error = "Resource file not found: " + filename;
    return false;
  }
  if (it->second.empty()) {
    error = "Resource is empty: " + filename;
    return false;
  }
  content = it->second;
  error.clear();
  return true;
}

bool InMemoryResourceContentSource::exists(
    const std::string& filename) const {
  return binaryContents.find(filename) != binaryContents.end() ||
         textContents.find(filename) != textContents.end();
}

bool InMemoryResourceContentSource::readText(const std::string& filename,
                                             std::string& content,
                                             std::string& error) {
  auto it = textContents.find(filename);
  if (it == textContents.end()) {
    error = "Resource file not found: " + filename;
    return false;
  }
  content = it->second;
  error.clear();
  return true;
}
