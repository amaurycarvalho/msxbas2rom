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
    LogLevel severity;
    string message;
    string file;
    int line;
    int column;
    int code;

    string toString() const;
  };

 private:
  vector<LogEntry> logs;
  LogEntry dummy;

  LogEntry& add(LogLevel level, const string& msg);
  string trim(const string& str) const;

 public:
  void clear();
  bool empty() const;

  LogEntry& info(const string& msg);
  LogEntry& warning(const string& msg);
  LogEntry& debug(const string& msg);
  LogEntry& error(const string& msg);

  LogEntry& add(LogEntry entry);

  bool contain(const set<LogLevel>& levels) const;
  bool containWarnings() const;
  bool containErrors() const;

  string toString() const;
  Logger errors() const;

  vector<LogEntry>& getAll();

  Logger filter(const set<LogLevel>& levels) const;
};

#endif