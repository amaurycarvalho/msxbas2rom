/***
 * @file build_path_policy.cpp
 * @brief Pure path policy used to derive build output names
 * @author Amaury Carvalho (2026)
 */

#include "build_path_policy.h"

namespace build_path {

#ifdef _WIN32
static const char folderSeparator = '\\';
#else
static const char folderSeparator = '/';
#endif

std::string getFilePath(const std::string& filepath) {
  std::size_t pos = filepath.find_last_of(folderSeparator);
  if (pos == std::string::npos) return "";
  return filepath.substr(0, pos);
}

std::string getFileName(const std::string& filepath) {
  std::size_t pos = filepath.find_last_of(folderSeparator);
  if (pos == std::string::npos) return filepath;
  return filepath.substr(pos + 1);
}

std::string getFileExtension(const std::string& filepath) {
  std::string filename = getFileName(filepath);
  std::size_t pos = filename.find_last_of('.');
  if (pos == std::string::npos) return "";
  return filename.substr(pos);
}

std::string getFileNameWithoutExtension(const std::string& filepath) {
  std::string filename = getFileName(filepath);
  std::size_t pos = filename.find_last_of('.');
  if (pos == std::string::npos) return filename;
  return filename.substr(0, pos);
}

std::string removeQuotes(const std::string& text) {
  std::string value = text;
  if (!value.empty() && value.front() == '"') value.erase(value.begin());
  if (!value.empty() && value.back() == '"') value.pop_back();
  return value;
}

std::string pathJoin(const std::string& base, const std::string& relative) {
  if (base.empty()) return relative;
  if (relative.empty()) return base;

  std::string b = base;
  std::string r = relative;

  if (!b.empty() && b.back() == folderSeparator) b.pop_back();
  if (!r.empty() && r.front() == folderSeparator) r.erase(0, 1);

  return b + folderSeparator + r;
}

}  // namespace build_path
