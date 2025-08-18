/***
 * @file cliparser.h
 * @brief Command Line Parameters Parser Header
 */

#ifndef COMMANDLINEPARSER_HPP
#define COMMANDLINEPARSER_HPP

#include <functional>
#include <string>
#include <unordered_map>

class CommandLineParser {
 public:
  struct Option {
    std::string longName;
    std::string description;
    bool requiresValue;
    bool deprecated;
    std::function<void(const std::string&)> handler;
  };

  void addOption(const std::string& shortName, const std::string& longName,
                 const std::string& description, bool requiresValue,
                 bool deprecated,
                 std::function<void(const std::string&)> handler);

  void parse(int argc, char* argv[]);
  std::string getFilename() const;
  void printHelp(const std::string& appName) const;

 private:
  std::unordered_map<std::string, Option> options_;
  std::string filename_;
};

#endif  // COMMANDLINEPARSER_HPP
