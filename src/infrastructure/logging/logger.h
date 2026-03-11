#ifndef LOGGER_H
#define LOGGER_H

#include <memory>
#include <set>
#include <string>
#include <vector>

using namespace std;

class Logger {
 public:
  enum class LogLevel { INFO, WARNING, DEBUG, ERROR };

  class LogEntry {
   private:
    string format(const LogEntry& entry) const;
    string levelToString(LogLevel level) const;

   public:
    LogLevel level;
    string message;

    string toString() const;
  };

 private:
  vector<LogEntry> logs;

  void add(LogLevel level, const string& msg);

 public:
  void clear();
  bool empty() const;

  void info(const string& msg);
  void warning(const string& msg);
  void debug(const string& msg);
  void error(const string& msg);

  void add(LogEntry entry);

  bool contain(const set<LogLevel>& levels) const;
  bool containWarnings() const;
  bool containErrors() const;

  string toString();

  vector<LogEntry>& getAll();

  Logger filter(const set<LogLevel>& levels) const;
};

#endif