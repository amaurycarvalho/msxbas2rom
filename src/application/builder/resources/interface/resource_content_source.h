/***
 * @file resource_content_source.h
 * @brief Application resource-content source port
 * @author Amaury Carvalho (2026)
 */

#ifndef RESOURCE_CONTENT_SOURCE_H_INCLUDED
#define RESOURCE_CONTENT_SOURCE_H_INCLUDED

#include <memory>
#include <string>
#include <vector>

/***
 * @brief Read-only access to resource bytes and text.
 */
class ResourceContentSource {
 public:
  virtual ~ResourceContentSource() {}

  virtual bool readBinary(const std::string& filename,
                          std::vector<unsigned char>& content,
                          std::string& error) = 0;
  virtual bool readText(const std::string& filename, std::string& content,
                        std::string& error) = 0;
  virtual bool exists(const std::string& filename) const = 0;
};

#endif  // RESOURCE_CONTENT_SOURCE_H_INCLUDED
