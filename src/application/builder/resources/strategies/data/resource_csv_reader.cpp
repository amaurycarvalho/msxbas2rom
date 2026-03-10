/***
 * @file resource_csv_reader.cpp
 * @brief Resource CSV reader implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_csv_reader.h"

#include <algorithm>
#include <exception>
#include <string.h>
#include <strings.h>

ResourceCsvReader::ResourceCsvReader(string filename)
    : ResourceTxtReader(filename) {
  resourceType = 1;  //! CSV resource type
  isIntegerData = false;
  /// @todo Refactor it implementing resource block linked list
  has1stBlockAnd2ndBlockSegmentDisalignmentBug = true;
};

void ResourceCsvReader::addFields(string line) {
  enum class State { Normal, Quoted, QuoteInQuoted };
  string field;
  State state = State::Normal;

  auto pushField = [&]() {
    lineFields.back().push_back(fixFieldValue(field));
    field.clear();
  };

  lineFields.emplace_back(0);

  for (char ch : line) {
    switch (state) {
      case State::Normal:
        if (ch == '"') {
          state = State::Quoted;
        } else if (ch == ',' || ch == ';' || ch == '\b') {
          pushField();
        } else if (!(ch == ' ' && field.empty())) {
          field.push_back(ch);
        }
        break;

      case State::Quoted:
        if (ch == '"') {
          state = State::QuoteInQuoted;  // possible escaped quote or end of
                                         // quoted field
        } else {
          field.push_back(ch);
        }
        break;

      case State::QuoteInQuoted:
        if (ch == '"') {
          field.push_back('"');   // escaped quote
          state = State::Quoted;  // back to inside quoted field
        } else if (ch == ',' || ch == ';' || ch == '\b') {
          pushField();
          state = State::Normal;  // end of quoted field
        } else {
          // Any other character after closing quote is ignored (common CSV
          // behavior)
          state = State::Normal;
        }
        break;
    }
  }

  if (!field.empty()) {
    pushField();
  }

  return;
}

string ResourceCsvReader::fixFieldValue(string field) {
  string s = field;
  int k;

  if (!s.empty()) {
    // Remove leading quote
    if (s.front() == '"') {
      s.erase(s.begin());
      if (s.empty()) return s;
    }

    // Remove trailing quote
    if (s.back() == '"') {
      s.pop_back();
      if (s.empty()) return s;
    }

    /// convert non base 10 numbers to base 10
    if (s.front() == '&') {
      try {
        if (s[1] == 'h' || s[1] == 'H')
          k = stoi(s.substr(2), 0, 16);
        else if (s[1] == 'o' || s[1] == 'O')
          k = stoi(s.substr(2), 0, 8);
        else if (s[1] == 'b' || s[1] == 'B')
          k = stoi(s.substr(2), 0, 2);
        else
          k = 0;
      } catch (exception &e) {
        k = 0;
      }
      s = to_string(k);
    }
  }

  return s;
}

bool ResourceCsvReader::populateFields() {
  int lineCount, i;
  /// CSV resource data
  lineFields.clear();
  lineNumbers.clear();
  if (populateLines()) {
    lineCount = min((int)lines.size(), 0xFFFF);
    /// parse line and add fields
    for (i = 0; i < lineCount; i++) {
      addFields(lines[i]);
    }
    return true;
  }
  return false;
}

/// @todo fix 1st block segment disalignment bug
/// implementing resource block linked list
bool ResourceCsvReader::populateData() {
  int i, k, lineCount;
  int fieldCount, fieldSize, fieldValue;
  /// initialize
  data.clear();
  /// resource type
  data.emplace_back(3);
  data.back()[0] = resourceType;
  unpackedSize = 3;
  packedSize = 3;
  /// line count
  lineCount = lineFields.size();
  data.back()[1] = (lineCount & 0xFF);
  data.back()[2] = ((lineCount >> 8) & 0xFF);
  /// lines data group
  for (i = 0; i < lineCount; i++) {
    /// lines numbers map
    if (i < (int)lineNumbers.size()) {
      data[0].push_back(lineNumbers[i] & 0xFF);
      data[0].push_back((lineNumbers[i] >> 8) & 0xFF);
      unpackedSize += 2;
      packedSize += 2;
    }
    /// lines fields map
    fieldCount = min((int)lineFields[i].size(), 0xFF);
    data[0].push_back(fieldCount);
    unpackedSize++;
    packedSize++;
    /// lines data
    for (k = 0; k < fieldCount; k++) {
      if (isIntegerData) {
        try {
          fieldValue = stoi(lineFields[i][k]);
        } catch (...) {
          fieldValue = 0;
        }
        data.emplace_back(2);
        data.back()[0] = (fieldValue & 0xFF);
        data.back()[1] = ((fieldValue >> 8) & 0xFF);
        unpackedSize += 2;
        packedSize += 2;
      } else {
        /// field maximum size = 255 chars
        fieldSize = min((int)lineFields[i][k].size(), 0xFF);
        /// set field size
        data.emplace_back(fieldSize + 1);
        data.back()[0] = fieldSize;
        /// copy string text (max = 255 chars)
        memcpy(data.back().data() + 1, lineFields[i][k].c_str(), fieldSize);
        unpackedSize += fieldSize + 1;
        packedSize += fieldSize + 1;
      }
    }
  }
  return true;
}

bool ResourceCsvReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".CSV") == 0);
}

bool ResourceCsvReader::load() {
  if (populateFields()) {
    return populateData();
  }
  return false;
}
