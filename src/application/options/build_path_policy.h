/***
 * @file build_path_policy.h
 * @brief Pure path policy used to derive build output names
 * @author Amaury Carvalho (2026)
 */

#ifndef BUILD_PATH_POLICY_H_INCLUDED
#define BUILD_PATH_POLICY_H_INCLUDED

#include <string>

namespace build_path {

std::string getFilePath(const std::string& filepath);
std::string getFileName(const std::string& filepath);
std::string getFileExtension(const std::string& filepath);
std::string getFileNameWithoutExtension(const std::string& filepath);
std::string pathJoin(const std::string& base, const std::string& relative);
std::string removeQuotes(const std::string& text);

}  // namespace build_path

#endif  // BUILD_PATH_POLICY_H_INCLUDED
