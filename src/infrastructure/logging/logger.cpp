#include "logger.h"

#include <algorithm>

void Logger::clear() {
  logs.clear();
}

bool Logger::empty() const {
  return logs.empty();
}

int Logger::size() const {
  return logs.size();
}

Logger::LogEntry& Logger::add(LogLevel severity, const string& message) {
  if (trim(message).empty()) return dummy;
  logs.push_back({severity, message});
  return logs.back();
}

Logger::LogEntry& Logger::add(LogEntry entry) {
  logs.push_back(entry);
  return logs.back();
}

string Logger::trim(const string& str) const {
  auto start = find_if_not(str.begin(), str.end(),
                           [](unsigned char c) { return isspace(c); });

  auto end = find_if_not(str.rbegin(), str.rend(), [](unsigned char c) {
               return isspace(c);
             }).base();

  if (start >= end) return "";

  return string(start, end);
}

bool Logger::contain(const set<LogLevel>& levels) const {
  for (const auto& log : logs) {
    if (levels.count(log.severity) > 0) {
      return true;
    }
  }
  return false;
}

bool Logger::containWarnings() const {
  return contain({LogLevel::WARNING});
}

bool Logger::containErrors() const {
  return contain({LogLevel::ERROR});
}

Logger Logger::errors() const {
  return filter({LogLevel::ERROR});
}

Logger Logger::infos() const {
  return filter({LogLevel::INFO});
}

Logger Logger::trace() const {
  return filter({LogLevel::INFO, LogLevel::ERROR});
}

void Logger::setFile(string file) {
  this->file = file;
}

string Logger::getFile() {
  return file;
}

void Logger::setLineNumber(int lineNumber) {
  this->lineNumber = lineNumber;
}

int Logger::getLineNumber() {
  return lineNumber;
}

string Logger::toString() const {
  string result = "";

  for (const auto& log : logs) {
    result += log.toString() + "\n";
  }

  return result;
}

Logger::LogEntry& Logger::info(const string& msg) {
  return add(LogLevel::INFO, msg);
}

Logger::LogEntry& Logger::warning(const string& msg) {
  return add(LogLevel::WARNING, msg);
}

Logger::LogEntry& Logger::debug(const string& msg) {
  return add(LogLevel::DEBUG, msg);
}

Logger::LogEntry& Logger::error(const string& msg) {
  auto& entry = add(LogLevel::ERROR, msg);
  entry.file = file;
  entry.line = lineNumber;
  return entry;
}

vector<Logger::LogEntry>& Logger::getAll() {
  return logs;
}

Logger Logger::filter(const set<LogLevel>& levels) const {
  Logger result;

  for (const auto& log : logs) {
    if (levels.count(log.severity)) {
      result.add(log);
    }
  }

  return result;
}

string Logger::LogEntry::levelToString(LogLevel level) const {
  switch (level) {
    case LogLevel::INFO:
      return "INFO";
    case LogLevel::WARNING:
      return "WARNING";
    case LogLevel::DEBUG:
      return "DEBUG";
    case LogLevel::ERROR:
      return "ERROR";
  }

  return "UNKNOWN";
}

string Logger::LogEntry::format(const LogEntry& entry) const {
  if (entry.severity == LogLevel::INFO) {
    return entry.message;
  } else if (!entry.file.empty()) {
    return entry.file + ":" + to_string(entry.line) + ": " +
           levelToString(entry.severity) + ": " + entry.message;
  }
  return levelToString(entry.severity) + ": " + entry.message;
}

string Logger::LogEntry::toString() const {
  return format(*this);
}
