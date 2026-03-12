/***
 * @file resource_data_reader.cpp
 * @brief Resource DATA reader implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_data_reader.h"

#include "logger.h"
#include "parser.h"

ResourceDataReader::ResourceDataReader(Parser* parser)
    : ResourceCsvReader(string("_DATA_")) {
  this->parser = parser;
  resourceType = 0;  //! DATA resource type
};

bool ResourceDataReader::load() {
  if (populateFields()) {
    return populateData();
  }
  return false;
}

bool ResourceDataReader::populateFields() {
  int fieldCount = parser->getDatas().size(), i;
  Lexeme* lexeme;
  string lineNumber;
  /// DATA/IDATA resource data
  lineFields.clear();
  lineNumbers.clear();
  for (i = 0; i < fieldCount; i++) {  // DATA/IDATA items values
    if ((lexeme = parser->getDatas()[i])) {
      if ((isIntegerData && lexeme->subtype == Lexeme::subtype_integer_data) ||
          (!isIntegerData && lexeme->subtype != Lexeme::subtype_integer_data)) {
        if (lineNumber != lexeme->tag) {
          lineNumber = lexeme->tag;
          lineFields.emplace_back(0);
          try {
            lineNumbers.push_back(stoi(lineNumber));
          } catch (...) {
            logger->error("DATA/IDATA statement invalid line number: " +
                          lineNumber);
            return false;
          }
        }
        lineFields.back().push_back(fixFieldValue(lexeme->value));
      }
    }
  }
  return true;
}
