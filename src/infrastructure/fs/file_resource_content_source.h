/***
 * @file file_resource_content_source.h
 * @brief File-backed resource-content adapter
 * @author Amaury Carvalho (2026)
 */

#ifndef FILE_RESOURCE_CONTENT_SOURCE_H_INCLUDED
#define FILE_RESOURCE_CONTENT_SOURCE_H_INCLUDED

#include "resource_content_source.h"

class FileResourceContentSource : public ResourceContentSource {
 public:
  FileResourceContentSource() {}

  bool readBinary(const std::string& filename,
                  std::vector<unsigned char>& content,
                  std::string& error) override;
  bool readText(const std::string& filename, std::string& content,
                std::string& error) override;
  bool exists(const std::string& filename) const override;
};

#endif  // FILE_RESOURCE_CONTENT_SOURCE_H_INCLUDED
