/***
 * @file file_resource_content_source.cpp
 * @brief File-backed resource-content adapter implementation
 * @author Amaury Carvalho (2026)
 */

#include "file_resource_content_source.h"

#include <fstream>

bool FileResourceContentSource::readBinary(const std::string& filename,
                                           std::vector<unsigned char>& content,
                                           std::string& error) {
  std::ifstream file(filename.c_str(), std::ios::binary | std::ios::ate);
  if (!file) {
    error = "Resource file not found: " + filename;
    return false;
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  if (size <= 0) {
    error = "Resource is empty: " + filename;
    return false;
  }

  content.assign(static_cast<std::size_t>(size), 0);
  if (!file.read(reinterpret_cast<char*>(content.data()), size)) {
    error = "Error reading resource: " + filename;
    return false;
  }

  error.clear();
  return true;
}

bool FileResourceContentSource::exists(const std::string& filename) const {
  std::ifstream file(filename.c_str());
  return file.good();
}

bool FileResourceContentSource::readText(const std::string& filename,
                                         std::string& content,
                                         std::string& error) {
  std::ifstream file(filename.c_str());
  if (!file) {
    error = "Resource file not found: " + filename;
    return false;
  }

  content.assign((std::istreambuf_iterator<char>(file)),
                 std::istreambuf_iterator<char>());
  error.clear();
  return true;
}
