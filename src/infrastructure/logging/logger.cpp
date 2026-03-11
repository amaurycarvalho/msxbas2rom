#include "logger.h"

#include <sstream>

void Logger::clear() {
  logs.clear();
}

bool Logger::empty() const {
  return logs.empty();
}

void Logger::add(LogLevel level, const string& msg) {
  logs.push_back({level, msg});
}

void Logger::add(LogEntry entry) {
  logs.push_back(entry);
}

bool Logger::contain(const set<LogLevel>& levels) const {
  for (const auto& log : logs) {
    if (levels.count(log.level) > 0) {
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

string Logger::toString() {
  string result = "";

  for (const auto& log : logs) {
    result += log.toString() + "\n";
  }

  return result;
}

void Logger::info(const string& msg) {
  add(LogLevel::INFO, msg);
}

void Logger::warning(const string& msg) {
  add(LogLevel::WARNING, msg);
}

void Logger::debug(const string& msg) {
  add(LogLevel::DEBUG, msg);
}

void Logger::error(const string& msg) {
  add(LogLevel::ERROR, msg);
}

vector<Logger::LogEntry>& Logger::getAll() {
  return logs;
}

Logger Logger::filter(const set<LogLevel>& levels) const {
  Logger result;

  for (const auto& log : logs) {
    if (levels.count(log.level)) {
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
  ostringstream oss;

  oss << "[" << levelToString(entry.level) << "] " << entry.message;

  return oss.str();
}

string Logger::LogEntry::toString() const {
  return format(*this);
}
