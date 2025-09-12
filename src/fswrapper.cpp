/***
 * @file fswrapper.cpp
 * @brief File system wrapper implementation
 * @note Solves some CPP cross OS differences
 * @author Amaury Carvalho
 */

#include "fswrapper.h"

#include <cmath>

using namespace std;

bool fileExists(const string& filename) {
  FILE* file;
  if ((file = fopen(filename.c_str(), "r"))) {
    fclose(file);
    return true;
  }
  return false;
}

string getFilePath(const string& filepath) {
  size_t pos = filepath.find_last_of(fsFolderSeparator);
  if (pos == string::npos) return "";  // no directory part
  return filepath.substr(0, pos);
}

string getFileName(const string& filepath) {
  size_t pos = filepath.find_last_of(fsFolderSeparator);
  if (pos == string::npos) return filepath;  // just filename
  return filepath.substr(pos + 1);
}

/***
 * @example .txt
 */
string getFileExtension(const string& filepath) {
  string filename = getFileName(filepath);
  size_t pos = filename.find_last_of('.');
  if (pos == string::npos) return "";  // no extension
  return filename.substr(pos);
}

string getFileNameWithoutExtension(const string& filepath) {
  string filename = getFileName(filepath);
  size_t pos = filename.find_last_of('.');
  if (pos == string::npos) return filename;  // no extension
  return filename.substr(0, pos);
}

string pathJoin(const string& base, const string& relative) {
  if (base.empty()) return relative;
  if (relative.empty()) return base;

  string b = base;
  string r = relative;

  // Remove trailing '/' from base
  if (!b.empty() && b.back() == fsFolderSeparator) b.pop_back();

  // Remove leading '/' from relative
  if (!r.empty() && r.front() == fsFolderSeparator) r.erase(0, 1);

  return b + fsFolderSeparator + r;
}

string removeQuotes(const string& text) {
  string s = text;

  // Remove leading quote
  if (!s.empty() && s.front() == '"') {
    s.erase(s.begin());
  }

  // Remove trailing quote
  if (!s.empty() && s.back() == '"') {
    s.pop_back();
  }

  return s;
}

float roundTo(float value, int decimals) {
  float factor = pow(10.0f, decimals);
  return round(value * factor) / factor;
}