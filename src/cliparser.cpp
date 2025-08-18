/***
 * @file cliparser.cpp
 * @brief Command Line Parameters Parser Implementation
 */

#include "cliparser.h"

#include <iostream>
#include <stdexcept>

void CommandLineParser::addOption(
    const std::string& shortName, const std::string& longName,
    const std::string& description, bool requiresValue, bool deprecated,
    std::function<void(const std::string&)> handler) {
  Option opt;
  opt.longName = longName;
  opt.description = description;
  opt.requiresValue = requiresValue;
  opt.deprecated = deprecated;
  opt.handler = handler;

  if (!shortName.empty()) {
    options_[shortName] = opt;
  }
  if (!longName.empty()) {
    options_[longName] = opt;
  }
}

void CommandLineParser::parse(int argc, char* argv[]) {
  // if (argc < 2) {
  //   throw std::runtime_error("No input file provided.");
  // }

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (options_.count(arg)) {
      Option& opt = options_.at(arg);
      std::string value;

      if (opt.requiresValue) {
        if (i + 1 < argc) {
          value = argv[++i];
        } else {
          throw std::runtime_error("Missing value for option: " + arg);
        }
      }

      if (opt.deprecated) {
        std::cerr << "Warning: Option '" << arg << "' is deprecated.\n";
      }

      if (opt.handler) {
        opt.handler(value);
      }
    } else {
      if (filename_.empty() && arg.c_str()[0] != '-') {
        filename_ = arg;
      } else {
        throw std::runtime_error("Unexpected argument: " + arg);
      }
    }
  }

  // if (filename_.empty()) {
  //   throw std::runtime_error("No filename provided.");
  // }
}

std::string CommandLineParser::getFilename() const {
  return filename_;
}

void CommandLineParser::printHelp(const std::string& appName) const {
  std::cout << "Usage: " << appName << " [options] <filename>\n\n";
  std::cout << "Options:\n";

  for (std::unordered_map<std::string, Option>::const_iterator it =
           options_.begin();
       it != options_.end(); ++it) {
    const std::string& flag = it->first;
    const Option& opt = it->second;

    // only print once for each option (skip duplicate longName entries)
    if (flag == opt.longName || flag.empty()) continue;

    std::cout << "  " << flag;
    if (!opt.longName.empty()) std::cout << ", " << opt.longName;
    if (opt.requiresValue) std::cout << " <value>";
    std::cout << " : " << opt.description;
    if (opt.deprecated) std::cout << " [DEPRECATED]";
    std::cout << "\n";
  }
}
