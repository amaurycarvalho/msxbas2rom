/***
 * @file resource_reader.h
 * @brief Resource reader base class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCE_READER_H_INCLUDED
#define RESOURCE_READER_H_INCLUDED

#include <string>
#include <vector>

using namespace std;

/***
 * @class ResourceReader
 * @brief Resource reader base class (abstract)
 */
class ResourceReader {
 protected:
  string filename;
  string errorMessage;

 public:
  vector<vector<unsigned char>> data;
  int unpackedSize;
  int packedSize;
  bool isPacked;
  bool has1stBlockAnd2ndBlockSegmentDisalignmentBug;
  const string getErrorMessage();
  const string getFilename();
  virtual bool remapTo(int index, int mappedSegm, int mappedAddress);
  virtual bool load() = 0;
  ResourceReader(string filename);
  virtual ~ResourceReader();
};

#endif  // RESOURCE_READER_H_INCLUDED
