/***
 * @file in_memory_resource_content_source.h
 * @brief In-memory resource-content source for tests
 * @author Amaury Carvalho (2026)
 */

#ifndef IN_MEMORY_RESOURCE_CONTENT_SOURCE_H_INCLUDED
#define IN_MEMORY_RESOURCE_CONTENT_SOURCE_H_INCLUDED

#include <map>
#include <string>
#include <vector>

#include "resource_content_source.h"

class InMemoryResourceContentSource : public ResourceContentSource {
 private:
  std::map<std::string, std::vector<unsigned char>> binaryContents;
  std::map<std::string, std::string> textContents;

 public:
  InMemoryResourceContentSource() {}

  void addBinary(const std::string& filename,
                 const std::vector<unsigned char>& content);
  void addText(const std::string& filename, const std::string& content);
  void clear();

  bool readBinary(const std::string& filename,
                  std::vector<unsigned char>& content,
                  std::string& error) override;
  bool readText(const std::string& filename, std::string& content,
                std::string& error) override;
  bool exists(const std::string& filename) const override;
};

#endif  // IN_MEMORY_RESOURCE_CONTENT_SOURCE_H_INCLUDED
