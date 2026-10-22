/***
 * @file build_options_resolver.h
 * @brief Application policy that derives output names from build options
 * @author Amaury Carvalho (2026)
 */

#ifndef BUILD_OPTIONS_RESOLVER_H_INCLUDED
#define BUILD_OPTIONS_RESOLVER_H_INCLUDED

#include <memory>
#include <string>

class BuildOptions;

namespace build_options {

void setInputFilename(BuildOptions& opts, const std::string& filename);
void setInputFilename(std::shared_ptr<BuildOptions> opts,
                      const std::string& filename);

}  // namespace build_options

#endif  // BUILD_OPTIONS_RESOLVER_H_INCLUDED
