/***
 * @file source_document.h
 * @brief Application source document value object
 * @author Amaury Carvalho (2026)
 */

#ifndef SOURCE_DOCUMENT_H_INCLUDED
#define SOURCE_DOCUMENT_H_INCLUDED

#include <string>
#include <vector>

/***
 * @brief A logical source name and its text content.
 */
class SourceDocument {
 public:
  std::string logicalName;
  std::string content;

  SourceDocument() {}
  SourceDocument(const std::string& logicalName, const std::string& content)
      : logicalName(logicalName), content(content) {}
};

/***
 * @brief Split source text using the historical 254-byte/line limit.
 */
std::vector<std::string> splitSourceLines(const std::string& content);

#endif  // SOURCE_DOCUMENT_H_INCLUDED
