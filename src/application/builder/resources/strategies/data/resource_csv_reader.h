/***
 * @file resource_csv_reader.h
 * @brief Resource CSV reader header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCE_CSV_READER_H_INCLUDED
#define RESOURCE_CSV_READER_H_INCLUDED

#include "resource_txt_reader.h"

/***
 * @class ResourceCsvReader
 * @brief Resource reader for CSV files
 * @remark
 *   CSV resource structure:
 *     resourceType N(1) = 1
 *     lineCount N(2)
 *     linesMap:
 *       lineFieldCount N(1)
 *     lineList:
 *       fieldList:
 *         fieldSize N(1)
 *         fieldData C(fieldSize)
 */
class ResourceCsvReader : public ResourceTxtReader {
 protected:
  int resourceType;
  bool isIntegerData;
  vector<int> lineNumbers;
  vector<vector<string>> lineFields;

  void addFields(string line);
  string fixFieldValue(string field);

  virtual bool populateFields();
  bool populateData();

 public:
  static bool isIt(string fileext);
  bool load();
  ResourceCsvReader(string filename);
};

#endif  // RESOURCE_CSV_READER_H_INCLUDED
