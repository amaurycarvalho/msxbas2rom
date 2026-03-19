#ifndef LOGGER_H
#define LOGGER_H

#include <memory>
#include <set>
#include <string>
#include <vector>

using namespace std;

class Logger {
 public:
  Logger();
  ~Logger();

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
  string file;
  int lineNumber = 0;

  LogEntry& add(LogLevel severity, const string& message);
  string trim(const string& str) const;

 public:
  void clear();
  bool empty() const;
  int size() const;

  LogEntry& info(const string& msg);
  LogEntry& warning(const string& msg);
  LogEntry& debug(const string& msg);
  LogEntry& error(const string& msg);

  LogEntry& add(LogEntry entry);
  void add(shared_ptr<Logger> logger);

  bool contain(const set<LogLevel>& levels) const;
  bool containWarnings() const;
  bool containErrors() const;

  string toString() const;
  Logger errors() const;
  Logger infos() const;
  Logger trace() const;

  void setFile(string file);
  string getFile();

  void setLineNumber(int lineNumber);
  int getLineNumber();

  vector<LogEntry>& getAll();

  Logger filter(const set<LogLevel>& levels) const;
};

#endif
